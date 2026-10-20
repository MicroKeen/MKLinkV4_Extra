#include "fish_media.h"
#include "fish_resources.h"
enum {COPY_QUIET_MS=3000,FINAL_QUIET_MS=1500,PROBE_RETRY_MS=3000};
static fish_media_phase_t state=FISH_MEDIA_READY;
static bool visible,probing;
static unsigned map;
static uint32_t last_write,retry_at,settled;
static uint32_t received,started,last_tick;
static fish_res_error_t error;
static bool diagnostic_acknowledged;
static void probe_end(void)
{
    fish_resources_cancel();
    if(probing)fish_media_port_probe(false);
    probing=false;
}
static void check_begin(void) {map=0;fish_resources_check_begin(0);}
void fish_media_begin(uint32_t now)
{
    visible=probing=false;last_write=retry_at=started=last_tick=now;received=0;
    error=FISH_RES_ERR_NONE;
    fish_fault_info_t fault;fish_media_port_reset_info(&fault);
    if(!diagnostic_acknowledged && fish_media_port_watchdog_reset() &&
       (fish_media_port_reset_stage() || fault.cause)) {state=FISH_MEDIA_FAULT;return;}
    state=FISH_MEDIA_CHECK;check_begin();
}
void fish_media_retry(uint32_t now)
{
    if(state!=FISH_MEDIA_FAULT)return;
    diagnostic_acknowledged=true;fish_media_begin(now);
}
void fish_media_require(uint32_t now)
{
    error=fish_resources_error();probe_end();last_write=retry_at=started=last_tick=now;
    received=0;state=FISH_MEDIA_WAIT;
}
void fish_media_host_write(uint32_t now,uint32_t bytes)
{
    if(!visible)return;
    probe_end();last_write=last_tick=retry_at=now;error=FISH_RES_ERR_NONE;state=FISH_MEDIA_WAIT;
    received=bytes>UINT32_MAX-received?UINT32_MAX:received+bytes;
}
void fish_media_tick(uint32_t now,bool save_busy)
{
    last_tick=now;
    if(state==FISH_MEDIA_READY || state==FISH_MEDIA_ERROR || state==FISH_MEDIA_FAULT)return;
    if(state==FISH_MEDIA_WAIT) {
        if(!visible) {
            if(save_busy)return;
            if(fish_media_port_start()) {state=FISH_MEDIA_ERROR;return;}
            visible=true;last_write=retry_at=now;return;
        }
        if((uint32_t)(now-last_write)<COPY_QUIET_MS || (uint32_t)(now-retry_at)<PROBE_RETRY_MS)return;
        if(fish_media_port_probe(true)) {retry_at=now;return;}
        probing=true;state=FISH_MEDIA_VERIFY;check_begin();
    }
    if(state==FISH_MEDIA_CHECK || state==FISH_MEDIA_VERIFY) {
        fish_resources_tick();
        if(fish_resources_phase()==FISH_RES_ERROR) {
            error=fish_resources_error();probe_end();state=FISH_MEDIA_WAIT;retry_at=now;return;
        }
        if(fish_resources_phase()!=FISH_RES_DONE)return;
        if(++map<2) {fish_resources_check_begin(map);return;}
        probe_end();
        /* The extra quiet window protects host ownership, not a normal boot
         * with no USB disk. Destination rendering is handled by the scene. */
        if(!visible)state=FISH_MEDIA_READY;
        else {settled=now;state=FISH_MEDIA_SETTLE;}
    }
    if(state==FISH_MEDIA_SETTLE && (uint32_t)(now-settled)>=FINAL_QUIET_MS &&
       (!visible || (uint32_t)(now-last_write)>=COPY_QUIET_MS)) {
        if(visible) {
            int result=fish_media_port_stop();
            if(result>0)return; /* Never cut an unfinished data/CSW transfer. */
            if(result<0) {state=FISH_MEDIA_ERROR;return;}
        }
        visible=false;state=FISH_MEDIA_READY;
    }
}
bool fish_media_ready(void) {return state==FISH_MEDIA_READY;}
bool fish_media_visible(void) {return visible;}
fish_media_phase_t fish_media_phase(void) {return state;}
unsigned fish_media_progress(void)
{
    if(state==FISH_MEDIA_READY || state==FISH_MEDIA_SETTLE)return 100;
    return state==FISH_MEDIA_CHECK || state==FISH_MEDIA_VERIFY?map*50+fish_resources_progress()/2:0;
}
fish_media_status_t fish_media_status(void)
{
    fish_media_status_t result={.phase=state,.progress=fish_media_progress(),
        .map=map<2?map:1,.received_bytes=received,.elapsed_ms=last_tick-started,
        .error=error,.receiving=visible && received && state==FISH_MEDIA_WAIT &&
            (uint32_t)(last_tick-last_write)<COPY_QUIET_MS,
        .watchdog_reset=fish_media_port_watchdog_reset(),.reset_stage=fish_media_port_reset_stage()};
    fish_media_port_reset_info(&result.fault);return result;
}
