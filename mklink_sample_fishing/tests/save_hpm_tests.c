/* Actual HPM save worker + FatFs on a synthetic RAM disk. No device I/O. */
#include "fish_game.h"
#include "fish_save.h"
#include "fish_fs_hpm.h"
#include "rtthread.h"
#include "ff.h"
#include "diskio.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t *card;
enum {SECTORS=32768};
static unsigned nesting,requests;
static uint32_t now;
static bool io_error;
static jmp_buf worker_return;
int rt_mutex_init(struct rt_mutex *m,const char *n,int f){(void)m;(void)n;(void)f;return 0;}
int rt_mutex_take(struct rt_mutex *m,int w){(void)m;(void)w;++nesting;return 0;}
int rt_mutex_release(struct rt_mutex *m){(void)m;assert(nesting);--nesting;return 0;}
int rt_sem_init(struct rt_semaphore *s,const char *n,unsigned v,int f){(void)s;(void)n;(void)v;(void)f;return 0;}
int rt_sem_take(struct rt_semaphore *s,int w){(void)s;(void)w;if(!requests)longjmp(worker_return,1);--requests;return 0;}
int rt_sem_release(struct rt_semaphore *s){(void)s;++requests;return 0;}
int rt_thread_init(struct rt_thread *t,const char *n,void (*fn)(void *),void *a,void *st,unsigned bytes,unsigned p,unsigned ticks)
{(void)t;(void)n;(void)fn;(void)a;(void)st;assert(bytes==4096);(void)p;(void)ticks;return 0;}
int rt_thread_startup(struct rt_thread *t){(void)t;return 0;}
rt_base_t rt_hw_interrupt_disable(void){return 0;}
void rt_hw_interrupt_enable(rt_base_t i){(void)i;}
int64_t get_system_time_ms(void){return now;}
int spi_sd_init(void){return 0;}
unsigned spi_sd_write_diagnostic(void){return 0;}
DSTATUS disk_initialize(BYTE d){assert(d==3);return 0;}
DSTATUS disk_status(BYTE d){assert(d==3);return 0;}
DRESULT disk_read(BYTE d,BYTE *p,LBA_t at,UINT n)
{assert(d==3 && at<SECTORS && n<=SECTORS-at);memcpy(p,card+(size_t)at*512,n*512);return RES_OK;}
DRESULT disk_write(BYTE d,const BYTE *p,LBA_t at,UINT n)
{assert(d==3 && nesting && at<SECTORS && n<=SECTORS-at);if(io_error)return RES_ERROR;memcpy(card+(size_t)at*512,p,n*512);return RES_OK;}
DRESULT disk_ioctl(BYTE d,BYTE op,void *p)
{assert(d==3);if(op==CTRL_SYNC)return io_error?RES_ERROR:RES_OK;if(op==GET_SECTOR_COUNT){*(LBA_t *)p=SECTORS;return RES_OK;}return RES_PARERR;}
#include "fish_save_hpm.c"
static void put16(uint8_t *p,unsigned n){p[0]=n;p[1]=n>>8;}
static void fixture(void)
{
    card=calloc(SECTORS,512);assert(card);uint8_t *p=card;
    p[0]=0xeb;p[1]=0x3c;p[2]=0x90;memcpy(p+3,"MSDOS5.0",8);
    put16(p+11,512);p[13]=2;put16(p+14,1);p[16]=2;put16(p+17,512);
    put16(p+19,SECTORS);p[21]=0xf8;put16(p+22,64);p[38]=0x29;
    memcpy(p+43,"MKLINK     ",11);memcpy(p+54,"FAT16   ",8);p[510]=0x55;p[511]=0xaa;
    for(unsigned i=0;i<2;i++){p=card+(1+i*64)*512;p[0]=0xf8;p[1]=p[2]=p[3]=0xff;}
}
static void save(fish_save_t *s,fish_game_t *g)
{
    g->save_requested=true;fish_save_tick(s,g,++now);
    assert(s->writing && requests);
    if(!setjmp(worker_return))save_worker(NULL);
    now+=401;fish_save_tick(s,g,now);assert(!s->writing && s->status==FISH_SAVE_OK && !nesting);
}
static void landed(fish_game_t *g,unsigned species,unsigned mm)
{
    g->mode=FISH_FIGHT;g->pending_species=species;g->pending_variant=0;
    g->pending_size_mm=mm;g->pending_weight_g=fish_species_weight_g(fish_species_profile(g->map_index,species),mm);
    g->reel_units=9999;g->tension=0;g->stamina=0;g->base_pull=0;g->line_strength=240;g->phase_at=g->simulation_at=now;
    fish_game_input(g,FISH_ROTATE,1,now);assert(g->mode==FISH_CAUGHT);
}
int main(void)
{
    fixture();fish_game_t g,loaded;fish_save_t s,restarted;
    fish_game_init(&g,now);fish_save_init(&s,&g,now);assert(!s.blocked);save(&s,&g);
    uint8_t boot[512];memcpy(boot,card,512);
    /* Existing downloader files must survive every game save and retry. */
    static const char original[]="MKLink downloader configuration: preserve exactly.";
    FIL existing;UINT count;assert(!fish_fs_lock());
    assert(f_open(&existing,"3:/MKLINK.BIN",FA_WRITE|FA_CREATE_NEW)==FR_OK);
    assert(f_write(&existing,original,sizeof(original),&count)==FR_OK && count==sizeof(original));
    assert(f_sync(&existing)==FR_OK && f_close(&existing)==FR_OK);fish_fs_unlock();
    g.economy.coins=10000;g.mode=FISH_LOADING;g.travel_target=1;g.travel_paid=g.travel_requested=true;
    fish_game_travel_complete(&g,true);assert(g.map_index==1);save(&s,&g);
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++){
        if(i==21)continue;
        const fish_species_t *p=fish_species_profile(1,i);
        landed(&g,i,p->max_mm);save(&s,&g);
        fish_game_init(&loaded,now);fish_save_init(&restarted,&loaded,now);
        if(restarted.blocked || loaded.map_index!=1 || loaded.last_species!=i){fprintf(stderr,"Failed after city species %u, status %d, seq %llu\n",i,restarted.status,(unsigned long long)restarted.sequence);abort();}
        assert(loaded.last_size_mm==p->max_mm && loaded.species_best_map[i]==1);
    }
    /* Fail an actual disk write, then retry without touching the active slot. */
    uint64_t sequence=s.sequence;g.economy.coins=1234;g.save_requested=true;
    fish_save_tick(&s,&g,++now);assert(s.writing);io_error=true;
    if(!setjmp(worker_return))save_worker(NULL);
    now+=401;fish_save_tick(&s,&g,now);io_error=false;
    assert(s.status==FISH_SAVE_ERROR && s.error_code==0x0801 && s.sequence==sequence);
    fish_game_init(&loaded,now);fish_save_init(&restarted,&loaded,now);
    assert(!restarted.blocked && restarted.sequence==sequence && loaded.economy.coins!=1234);
    save(&s,&g);fish_game_init(&loaded,now);fish_save_init(&restarted,&loaded,now);
    assert(!restarted.blocked && loaded.economy.coins==1234 && loaded.map_index==1);
    g.mode=FISH_SHORE;g.shore_x=FISH_CITY_STALL_X;g.boat_x=FISH_BOAT_LEFT;g.facing=-1;
    g.game_minute=1361;g.clock_remainder_ms=996;g.weather_ms=13612278;g.wetness_units=FISH_WETNESS_FULL;
    save(&s,&g);fish_game_init(&loaded,now);fish_save_init(&restarted,&loaded,now);
    assert(loaded.mode==FISH_SHORE && loaded.shore_x==g.shore_x && loaded.boat_x==g.boat_x && loaded.facing==-1);
    assert(loaded.game_minute==1361 && loaded.clock_remainder_ms==996 && loaded.weather_ms==13612278 && loaded.wetness_units==FISH_WETNESS_FULL);
    char preserved[sizeof(original)];assert(!fish_fs_lock());
    assert(f_open(&existing,"3:/MKLINK.BIN",FA_READ)==FR_OK && f_size(&existing)==sizeof(original));
    assert(f_read(&existing,preserved,sizeof(preserved),&count)==FR_OK && count==sizeof(preserved));
    assert(f_close(&existing)==FR_OK && !memcmp(original,preserved,sizeof(original)));fish_fs_unlock();
    assert(!memcmp(card,boot,512));free(card);
    puts("PASS: actual HPM worker/FatFs city travel, every fish landing, A/B writes, S08/01 recovery, exact position/time/weather reboot; existing downloader file and volume boot sector preserved");
}
