#include "fish_save.h"
#include "fish_save_crypto.h"
#include "fish_save_storage.h"
#include <string.h>
#include <limits.h>

static void put16(uint8_t *p,uint16_t x) {p[0]=(uint8_t)x;p[1]=(uint8_t)(x>>8);}
static uint16_t get16(const uint8_t *p) {return (uint16_t)(p[0]|((unsigned)p[1]<<8));}
static void put32(uint8_t *p,uint32_t x) {for(unsigned i=0;i<4;i++) p[i]=(uint8_t)(x>>(8*i));}
static uint32_t get32(const uint8_t *p) {uint32_t x=0;for(unsigned i=0;i<4;i++) x|=(uint32_t)p[i]<<(8*i);return x;}
static unsigned progress_offset(unsigned i) {return i<FISH_BASE_SPECIES_COUNT?16+4*i:204+14*(i-FISH_BASE_SPECIES_COUNT);}
static unsigned cargo_offset(unsigned i) {return i<FISH_BASE_SPECIES_COUNT?77+10*i:208+14*(i-FISH_BASE_SPECIES_COUNT);}
void fish_save_pack(const fish_game_t *g,uint8_t p[FISH_SAVE_PAYLOAD_BYTES])
{
    memset(p,0,FISH_SAVE_PAYLOAD_BYTES);
    p[0]=g->rod_index;p[1]=g->line_index;p[2]=g->float_index;p[3]=g->bait_index;p[4]=g->depth_index;
    p[5]=g->last_species;put16(p+6,g->last_size_mm);put32(p+8,(uint32_t)g->catches);
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=progress_offset(i);put16(p+at,g->species_catches[i]);put16(p+at+2,g->species_best_size_mm[i]);
    }
    const fish_economy_t *e=&g->economy;
    put32(p+60,e->coins);p[64]=e->rods;p[65]=e->lines;p[66]=e->floats;
    for(unsigned i=0;i<FISH_SPECIAL_BAIT_COUNT;i++) put16(p+67+2*i,e->bait_stock[i]);
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=cargo_offset(i);put16(p+at,e->count[i]);put32(p+at+2,e->grams[i]);put32(p+at+6,e->value[i]);
    }
    p[187]=e->hanging_species;put16(p+188,e->hanging_size_mm);
    put32(p+190,e->hanging_grams);put32(p+194,e->hanging_value);put16(p+198,g->game_minute);
    p[200]=e->container;p[201]=FISH_SAVE_EQUIPMENT_EPOCH;
    p[FISH_SAVE_V5_PAYLOAD_BYTES]=g->sound_mode;
}
bool fish_save_unpack(fish_game_t *g,const uint8_t p[FISH_SAVE_PAYLOAD_BYTES])
{
    if(p[0]>=FISH_ROD_COUNT || p[1]>=FISH_LINE_COUNT || p[2]>=FISH_FLOAT_COUNT ||
       p[3]>=FISH_BAIT_COUNT || p[4]>=FISH_DEPTH_OPTION_COUNT || p[5]>=FISH_SPECIES_COUNT ||
       get32(p+8)>INT_MAX || get32(p+12) || p[FISH_SAVE_V5_PAYLOAD_BYTES]>=FISH_SOUND_COUNT) return false;
    uint32_t total=0;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=progress_offset(i),count=get16(p+at),best=get16(p+at+2);total+=count;
        if((!count && best) || (count && (best<fish_species[i].min_mm || best>fish_species[i].max_mm))) return false;
    }
    unsigned last=get16(p+6);
    if(total>get32(p+8) || (get32(p+8) && (!get16(p+progress_offset(p[5])) ||
       last<fish_species[p[5]].min_mm || last>get16(p+progress_offset(p[5])+2))) || (!get32(p+8) && last)) return false;
    fish_economy_t e;fish_economy_init(&e);
    e.coins=get32(p+60);e.rods=p[64];e.lines=p[65];e.floats=p[66];
    if(e.coins>FISH_COIN_LIMIT || (e.rods&~7u) || (e.lines&~7u) || (e.floats&~7u) ||
       !(e.rods&1) || !(e.lines&2) || !(e.floats&1) ||
       !(e.rods&(1u<<p[0])) || !(e.lines&(1u<<p[1])) || !(e.floats&(1u<<p[2])) ||
       get16(p+198)>=1440 || p[200]>=FISH_CONTAINER_COUNT || p[201]>FISH_SAVE_EQUIPMENT_EPOCH || p[202] || p[203]) return false;
    e.container=p[200];
    for(unsigned i=0;i<FISH_SPECIAL_BAIT_COUNT;i++) {
        e.bait_stock[i]=get16(p+67+2*i);if(e.bait_stock[i]>FISH_BAIT_STOCK_LIMIT) return false;
    }
    uint64_t weight=0,value=0;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=cargo_offset(i);e.count[i]=get16(p+at);e.grams[i]=get32(p+at+2);e.value[i]=get32(p+at+6);
        if((!e.count[i] && (e.grams[i] || e.value[i])) ||
           (e.count[i] && (e.grams[i]<e.count[i] || e.value[i]<e.count[i])) ||
           e.value[i]>(uint64_t)e.count[i]*fish_catch_price(i,fish_species[i].max_mm)) return false;
        weight+=e.grams[i];value+=e.value[i];
    }
    e.hanging_species=p[187];e.hanging_size_mm=get16(p+188);
    e.hanging_grams=get32(p+190);e.hanging_value=get32(p+194);
    if(e.hanging_species>FISH_SPECIES_COUNT || weight>fish_hold_capacity(&e) || value>FISH_COIN_LIMIT) return false;
    if(e.hanging_species==FISH_SPECIES_COUNT) {
        if(e.hanging_size_mm || e.hanging_grams || e.hanging_value) return false;
    } else {
        const fish_species_t *s=&fish_species[e.hanging_species];
        if(e.hanging_size_mm<s->min_mm || e.hanging_size_mm>s->max_mm ||
           e.hanging_grams<=FISH_HOLD_CAPACITY_G ||
           e.hanging_grams!=fish_species_weight_g(s,e.hanging_size_mm) ||
           e.hanging_value!=fish_catch_price(e.hanging_species,e.hanging_size_mm)) return false;
    }
    /* Apply only after all validation; transient fishing/input state stays reset. */
    g->rod_index=p[0];fish_game_equip_line(g,p[1]);g->float_index=p[2];g->bait_index=p[3];g->depth_index=p[4];
    g->last_species=p[5];g->last_size_mm=(uint16_t)last;g->catches=(int)get32(p+8);
    g->last_weight_g=last?fish_species_weight_g(&fish_species[p[5]],(uint16_t)last):0;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=progress_offset(i);g->species_catches[i]=get16(p+at);g->species_best_size_mm[i]=get16(p+at+2);
    }
    g->economy=e;g->game_minute=get16(p+198);g->clock_remainder_ms=0;
    g->sound_mode=p[FISH_SAVE_V5_PAYLOAD_BYTES];
    if(!p[201]) {
        /* This release resets legacy equipment once; progress and cargo stay. */
        g->rod_index=FISH_DEFAULT_ROD;fish_game_equip_line(g,FISH_DEFAULT_LINE_SET);
        g->float_index=FISH_DEFAULT_FLOAT;g->bait_index=FISH_DEFAULT_BAIT;
        g->depth_index=FISH_DEPTH_OPTION_COUNT-1;
        g->economy.rods=1u<<FISH_DEFAULT_ROD;
        g->economy.lines=1u<<FISH_DEFAULT_LINE_SET;
        g->economy.floats=1u<<FISH_DEFAULT_FLOAT;
    }
    return true;
}
void fish_save_encode(const uint8_t key[32],uint64_t seq,const uint8_t *p,uint8_t *file)
{
    memcpy(file,"FSG1",4);put16(file+4,FISH_SAVE_VERSION);put16(file+6,FISH_SAVE_PAYLOAD_BYTES);
    for(unsigned i=0;i<8;i++) file[8+i]=(uint8_t)(seq>>(8*i));
    fish_save_seal(key,file,16,p,FISH_SAVE_PAYLOAD_BYTES,file+16,file+32);
}
bool fish_save_decode(const uint8_t key[32],const uint8_t *file,size_t len,uint64_t *seq,uint8_t *p)
{
    if(len!=FISH_SAVE_LEGACY_FILE_BYTES && len!=FISH_SAVE_PREVIOUS_FILE_BYTES && len!=FISH_SAVE_V5_FILE_BYTES && len!=FISH_SAVE_FILE_BYTES) return false;
    unsigned version=get16(file+4);bool legacy=version==1;
    size_t bytes=legacy?FISH_SAVE_LEGACY_PAYLOAD_BYTES:version<5?FISH_SAVE_PREVIOUS_PAYLOAD_BYTES:version==5?FISH_SAVE_V5_PAYLOAD_BYTES:FISH_SAVE_PAYLOAD_BYTES;
    if(version<1 || version>FISH_SAVE_VERSION || len!=bytes+32 || memcmp(file,"FSG1",4) || get16(file+6)!=bytes) return false;
    memset(p,0,FISH_SAVE_PAYLOAD_BYTES);
    if(!fish_save_open(key,file,16,file+16,file+32,bytes,p)) return false;
    if(version<6)p[FISH_SAVE_V5_PAYLOAD_BYTES]=FISH_SOUND_ALL;
    if(version==2 && get32(p+200)) return false;
    if(version==3 && (p[201] || p[202] || p[203])) return false;
    if(version>=4 && p[201]!=FISH_SAVE_EQUIPMENT_EPOCH) return false;
    if(version<5 && (p[5]>=FISH_BASE_SPECIES_COUNT || (!legacy && p[187]>FISH_BASE_SPECIES_COUNT))) return false;
    if(version<5 && p[187]==FISH_BASE_SPECIES_COUNT) p[187]=FISH_SPECIES_COUNT;
    uint64_t n=0;for(unsigned i=0;i<8;i++) n|=(uint64_t)file[8+i]<<(8*i);
    if(!n) return false;
    if(legacy) {
        /* Keep every legacy core byte/index, grant starter plus equipped items. */
        if(p[0]>=FISH_ROD_COUNT || p[1]>=FISH_LINE_COUNT || p[2]>=FISH_FLOAT_COUNT || p[3]>=FISH_SPECIAL_BAIT_COUNT) return false;
        p[64]=(uint8_t)(1u|(1u<<p[0]));p[65]=(uint8_t)(2u|(1u<<p[1]));p[66]=(uint8_t)(1u|(1u<<p[2]));
        put16(p+67+2*p[3],5);p[187]=FISH_SPECIES_COUNT;put16(p+198,360);
    }
    *seq=n;return true;
}
void fish_save_init(fish_save_t *s,fish_game_t *g,uint32_t now)
{
    memset(s,0,sizeof(*s));s->active_slot=-1;s->auto_at=now;s->status_at=now;
    fish_save_pack(g,s->saved);
    if(fish_storage_init(s->key)!=0) {s->status=s->blocked_status=FISH_SAVE_ERROR;s->blocked=true;return;}
    bool invalid=false,io=false,migrated=false,equipment_initialized=false;
    unsigned best_priority=0;
    for(unsigned slot=0;slot<6;slot++) {
        if((slot==2 && best_priority==2) || (slot==4 && best_priority>=1)) break;
        int result=fish_storage_read(slot,s->file,sizeof(s->file));
        size_t length=sizeof(s->file);
        if(result==-3) {length=FISH_SAVE_V5_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_PREVIOUS_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_LEGACY_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-2) continue;
        if(result==-1) {io=true;continue;}
        uint64_t seq;fish_game_t checked=*g;
        if(result || !fish_save_decode(s->key,s->file,length,&seq,s->pending) ||
           !fish_save_unpack(&checked,s->pending)) {invalid=true;continue;}
        equipment_initialized|=s->pending[201]==FISH_SAVE_EQUIPMENT_EPOCH;
        unsigned version=get16(s->file+4),priority=version==6?2:version==5?1:0;
        bool current=version==FISH_SAVE_VERSION;
        if(s->active_slot<0 || priority>best_priority || (priority==best_priority && seq>s->sequence)) {
            s->sequence=seq;s->active_slot=(int)(slot&1);memcpy(s->saved,s->pending,sizeof(s->saved));
            migrated=!current;best_priority=priority;
        }
    }
    if(s->active_slot>=0) {
        /* An older firmware may have written a newer sequence. A valid v4
         * slot still proves that this device already completed the reset. */
        if(equipment_initialized) s->saved[201]=FISH_SAVE_EQUIPMENT_EPOCH;
        fish_save_unpack(g,s->saved);
        if(migrated) g->save_requested=true;
        s->status=invalid?FISH_SAVE_RECOVERED:io?FISH_SAVE_ERROR:FISH_SAVE_IDLE;
        if(io) {s->blocked=true;s->blocked_status=FISH_SAVE_ERROR;}
    } else if(invalid || io) {
        s->status=s->blocked_status=invalid?FISH_SAVE_INVALID:FISH_SAVE_ERROR;s->blocked=true;
    }
}
void fish_save_tick(fish_save_t *s,fish_game_t *g,uint32_t now)
{
    s->manual_pending|=g->save_requested;g->save_requested=false;
    if(s->writing) {
        int result=fish_storage_poll();
        if(result) {
            s->writing=false;s->status_at=now;
            if(result>0) {
                s->active_slot=s->active_slot==0?1:0;++s->sequence;
                memcpy(s->saved,s->pending,sizeof(s->saved));s->status=FISH_SAVE_OK;
            } else s->status=FISH_SAVE_ERROR;
        }
        return;
    }
    bool manual=s->manual_pending;s->manual_pending=false;
    if(s->status!=FISH_SAVE_IDLE && now-s->status_at>=2500) s->status=FISH_SAVE_IDLE;
    bool due=now-s->auto_at>=FISH_SAVE_INTERVAL_MS;
    if(due) s->auto_at=now;
    if(!manual && (!due || s->blocked)) return;
    if(s->blocked) {s->status=s->blocked_status;s->status_at=now;return;}
    fish_save_pack(g,s->pending);
    if(!manual && !memcmp(s->saved,s->pending,sizeof(s->saved))) return;
    if(s->sequence==UINT64_MAX) {s->status=FISH_SAVE_ERROR;s->status_at=now;return;}
    fish_save_encode(s->key,s->sequence+1,s->pending,s->file);
    s->status_at=now;s->auto_at=now;
    if(fish_storage_begin(s->active_slot==0?1:0,s->file,sizeof(s->file))) s->status=FISH_SAVE_ERROR;
    else {s->status=FISH_SAVE_SAVING;s->writing=true;}
}
const char *fish_save_label(const fish_save_t *s)
{
    switch(s->status) {
        case FISH_SAVE_SAVING:return "存档中";
        case FISH_SAVE_OK:return "已存档";
        case FISH_SAVE_ERROR:return "存档失败";
        case FISH_SAVE_RECOVERED:return "存档恢复";
        case FISH_SAVE_INVALID:return "存档异常";
        default:return "";
    }
}
