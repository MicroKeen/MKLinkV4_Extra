#include "fish_save.h"
#include "fish_save_crypto.h"
#include "fish_save_storage.h"
#include <string.h>
#include <limits.h>

__attribute__((weak)) unsigned fish_storage_diagnostic(void) {return 0;}
__attribute__((weak)) unsigned fish_storage_write_diagnostic(void) {return 0;}

static void put16(uint8_t *p,uint16_t x) {p[0]=(uint8_t)x;p[1]=(uint8_t)(x>>8);}
static uint16_t get16(const uint8_t *p) {return (uint16_t)(p[0]|((unsigned)p[1]<<8));}
static void put32(uint8_t *p,uint32_t x) {for(unsigned i=0;i<4;i++) p[i]=(uint8_t)(x>>(8*i));}
static uint32_t get32(const uint8_t *p) {uint32_t x=0;for(unsigned i=0;i<4;i++) x|=(uint32_t)p[i]<<(8*i);return x;}
static unsigned progress_offset(unsigned i) {return i<11?16+4*i:i<15?204+14*(i-11):i==FISH_BIGHEAD?FISH_SAVE_BIGHEAD_OFFSET:FISH_SAVE_V8_PAYLOAD_BYTES+14*(i-15);}
static unsigned cargo_offset(unsigned i) {return i<11?77+10*i:i<15?208+14*(i-11):progress_offset(i)+4;}
static unsigned best_map_offset(unsigned i) {return i==FISH_BIGHEAD?FISH_SAVE_BIGHEAD_MAP_OFFSET:FISH_SAVE_BEST_MAP_OFFSET+i;}
void fish_save_pack(const fish_game_t *g,uint8_t p[FISH_SAVE_PAYLOAD_BYTES])
{
    memset(p,0,FISH_SAVE_PAYLOAD_BYTES);
    p[0]=g->rod_index;p[1]=g->line_index;p[2]=g->float_index;p[3]=g->bait_index;p[4]=g->depth_index;
    p[5]=g->last_species;put16(p+6,g->last_size_mm);put32(p+8,(uint32_t)g->catches);
    /* V13 uses six previously reserved bytes; file/buffer sizes stay unchanged. */
    put16(p+12,(uint16_t)g->boat_x);put16(p+14,(uint16_t)g->shore_x);
    bool ashore=fish_game_ashore(g) || (g->mode==FISH_LOADING && (g->travel_paid || g->previous_mode==FISH_SHORE));
    p[202]=(uint8_t)(0x80|(ashore?1:0)|(g->facing<0?2:0));p[203]=(uint8_t)(g->clock_remainder_ms/4);
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=progress_offset(i);put16(p+at,g->species_catches[i]);put16(p+at+2,g->species_best_size_mm[i]);
    }
    const fish_economy_t *e=&g->economy;
    for(unsigned i=0;i<3;i++)put16(p+FISH_SAVE_CITY_BAIT_OFFSET+2*i,e->bait_stock[FISH_BAIT_SNAIL+i]);
    put32(p+FISH_SAVE_LATTE_OFFSET,g->latte_remaining_ms);
    put32(p+FISH_SAVE_WEATHER_OFFSET,g->weather_ms);put32(p+FISH_SAVE_WETNESS_OFFSET,g->wetness_units);
    put32(p+60,e->coins);p[64]=e->rods;p[65]=e->lines;p[66]=e->floats;
    for(unsigned i=0;i<FISH_SPECIAL_BAIT_COUNT;i++) put16(p+67+2*i,e->bait_stock[i]);
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=cargo_offset(i);put16(p+at,e->count[i]);put32(p+at+2,e->grams[i]);put32(p+at+6,e->value[i]);
    }
    p[187]=e->hanging_species;put16(p+188,e->hanging_size_mm);
    put32(p+190,e->hanging_grams);put32(p+194,e->hanging_value);put16(p+198,g->game_minute);
    p[200]=e->container;p[201]=FISH_SAVE_EQUIPMENT_EPOCH;
    p[FISH_SAVE_V7_PAYLOAD_BYTES]=g->map_index;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++)p[best_map_offset(i)]=g->species_best_map[i];
    p[FISH_SAVE_HANGING_MAP_OFFSET]=e->hanging_species<FISH_SPECIES_COUNT?e->hanging_map:0;
    p[FISH_SAVE_LAST_MAP_OFFSET]=g->last_map;
    p[FISH_SAVE_V5_PAYLOAD_BYTES]=g->sound_mode;
    for(unsigned i=0;i<FISH_PRICE_GROUPS;i++) {
        put16(p+FISH_SAVE_V6_PAYLOAD_BYTES+4*i,e->prices[i].price);
        put16(p+FISH_SAVE_V6_PAYLOAD_BYTES+4*i+2,e->prices[i].count);
    }
}
static bool unpack(fish_game_t *g,const uint8_t p[FISH_SAVE_PAYLOAD_BYTES])
{
    if(p[0]>=FISH_ROD_COUNT || p[1]>=FISH_LINE_COUNT || p[2]>=FISH_FLOAT_COUNT ||
       p[3]>=FISH_BAIT_COUNT || p[4]>=FISH_DEPTH_OPTION_COUNT || p[5]>=FISH_SPECIES_COUNT ||
       p[FISH_SAVE_V7_PAYLOAD_BYTES]>=FISH_MAP_COUNT || p[FISH_SAVE_LAST_MAP_OFFSET]>=3 || p[FISH_SAVE_HANGING_MAP_OFFSET]>=3 || get32(p+8)>INT_MAX || p[FISH_SAVE_V5_PAYLOAD_BYTES]>=FISH_SOUND_COUNT) return false;
    bool location=p[202]&0x80;int boat=get16(p+12),shore=(int16_t)get16(p+14);
    if(location) {
        if((p[202]&~0x83u) || p[203]>249 || boat<FISH_BOAT_LEFT || boat>FISH_BOAT_RIGHT ||
           shore<FISH_SHORE_LEFT || shore>fish_docks[1].walk_right ||
           ((p[202]&1) && shore>fish_docks[0].walk_right && shore<fish_docks[1].walk_left))return false;
    } else if(get32(p+12) || p[202] || p[203])return false;
    if(get32(p+FISH_SAVE_WEATHER_OFFSET)>=FISH_WEATHER_CYCLE_MS || get32(p+FISH_SAVE_WETNESS_OFFSET)>FISH_WETNESS_FULL)return false;
    uint32_t total=0;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=progress_offset(i),count=get16(p+at),best=get16(p+at+2);total+=count;
        unsigned origin=p[best_map_offset(i)];if(origin>=3)return false;
        const fish_species_t *profile=fish_species_profile(origin,i);
        if((!count && best) || (count && (best<profile->min_mm || best>profile->max_mm))) return false;
    }
    unsigned last=get16(p+6);
    if(total>get32(p+8) || (get32(p+8) && (!get16(p+progress_offset(p[5])) ||
       last<fish_species_profile(p[FISH_SAVE_LAST_MAP_OFFSET],p[5])->min_mm || last>fish_species_profile(p[FISH_SAVE_LAST_MAP_OFFSET],p[5])->max_mm || last>get16(p+progress_offset(p[5])+2))) || (!get32(p+8) && last)) return false;
    fish_economy_t e;fish_economy_init(&e);
    e.coins=get32(p+60);e.rods=p[64];e.lines=p[65];e.floats=p[66];
    if(e.coins>FISH_COIN_LIMIT || (e.rods&~7u) || (e.lines&~((1u<<FISH_LINE_COUNT)-1)) || (e.floats&~7u) ||
       !(e.rods&1) || !(e.floats&1) ||
       !(e.rods&(1u<<p[0])) || (e.lines && !(e.lines&(1u<<p[1]))) || !(e.floats&(1u<<p[2])) ||
       get16(p+198)>=1440 || p[200]>=FISH_CONTAINER_COUNT || p[201]>FISH_SAVE_EQUIPMENT_EPOCH) return false;
    e.container=p[200];
    for(unsigned i=0;i<FISH_SPECIAL_BAIT_COUNT;i++) {
        e.bait_stock[i]=get16(p+67+2*i);if(e.bait_stock[i]>FISH_BAIT_STOCK_LIMIT) return false;
    }
    for(unsigned i=0;i<3;i++) {
        e.bait_stock[FISH_BAIT_SNAIL+i]=get16(p+FISH_SAVE_CITY_BAIT_OFFSET+2*i);
        if(e.bait_stock[FISH_BAIT_SNAIL+i]>FISH_BAIT_STOCK_LIMIT)return false;
    }
    if(get32(p+FISH_SAVE_LATTE_OFFSET)>FISH_LATTE_DURATION_MS)return false;
    uint64_t weight=0,value=0;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=cargo_offset(i);e.count[i]=get16(p+at);e.grams[i]=get32(p+at+2);e.value[i]=get32(p+at+6);
        uint32_t limit=fish_catch_price(i,fish_species[i].max_mm);
        uint32_t city=fish_catch_price_on(1,i,fish_species_profile(1,i)->max_mm);if(city>limit)limit=city;
        uint32_t old=fish_catch_price_on(2,i,fish_species_profile(2,i)->max_mm);if(old>limit)limit=old;
        if((!e.count[i] && (e.grams[i] || e.value[i])) ||
           (e.count[i] && (e.grams[i]<e.count[i] || e.value[i]<e.count[i])) ||
           e.value[i]>(uint64_t)e.count[i]*limit) return false;
        weight+=e.grams[i];value+=e.value[i];
    }
    e.hanging_species=p[187];e.hanging_size_mm=get16(p+188);
    e.hanging_map=p[FISH_SAVE_HANGING_MAP_OFFSET];
    e.hanging_grams=get32(p+190);e.hanging_value=get32(p+194);
    if(e.hanging_species>FISH_SPECIES_COUNT || weight>fish_hold_capacity(&e) || value>FISH_COIN_LIMIT) return false;
    if(e.hanging_species==FISH_SPECIES_COUNT) {
        if(e.hanging_size_mm || e.hanging_grams || e.hanging_value) return false;
    } else {
        const fish_species_t *s=fish_species_profile(e.hanging_map,e.hanging_species);
        if(e.hanging_size_mm<s->min_mm || e.hanging_size_mm>s->max_mm ||
           e.hanging_grams<=FISH_HOLD_CAPACITY_G ||
           e.hanging_grams!=fish_species_weight_g(s,e.hanging_size_mm) ||
           e.hanging_value!=fish_catch_price_on(e.hanging_map,e.hanging_species,e.hanging_size_mm)) return false;
    }
    uint64_t detailed=0;uint32_t detailed_count=0,cargo_count=0;bool end=false;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) cargo_count+=e.count[i];
    for(unsigned i=0;i<FISH_PRICE_GROUPS;i++) {
        fish_price_group_t *r=&e.prices[i];
        r->price=get16(p+FISH_SAVE_V6_PAYLOAD_BYTES+4*i);r->count=get16(p+FISH_SAVE_V6_PAYLOAD_BYTES+4*i+2);
        if(!r->count) {if(r->price) return false;end=true;continue;}
        if(end || !r->price) return false;
        for(unsigned j=0;j<i;j++) if(e.prices[j].price==r->price) return false;
        detailed+=(uint32_t)r->price*r->count;detailed_count+=r->count;
    }
    if(detailed>value || detailed_count>cargo_count ||
       (detailed_count==cargo_count && detailed!=value) || value-detailed<cargo_count-detailed_count) return false;
    /* NULL is the boot candidate validation path; avoid a second full game on
     * the scene thread's stack now that cargo has an exact-price journal. */
    if(!g) return true;
    /* Apply only after all validation; transient fishing/input state stays reset. */
    g->rod_index=p[0];fish_game_equip_line(g,p[1]);g->float_index=p[2];g->bait_index=p[3];g->depth_index=p[4];
    g->last_species=p[5];g->last_size_mm=(uint16_t)last;g->catches=(int)get32(p+8);
    g->last_map=p[FISH_SAVE_LAST_MAP_OFFSET];
    g->last_weight_g=last?fish_species_weight_g(fish_species_profile(g->last_map,p[5]),(uint16_t)last):0;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++)g->species_best_map[i]=p[best_map_offset(i)];
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned at=progress_offset(i);g->species_catches[i]=get16(p+at);g->species_best_size_mm[i]=get16(p+at+2);
    }
    g->latte_remaining_ms=get32(p+FISH_SAVE_LATTE_OFFSET);
    g->weather_ms=get32(p+FISH_SAVE_WEATHER_OFFSET);g->wetness_units=get32(p+FISH_SAVE_WETNESS_OFFSET);
    g->economy=e;g->game_minute=get16(p+198);g->clock_remainder_ms=0;
    g->map_index=p[FISH_SAVE_V7_PAYLOAD_BYTES];
    if(location) {
        g->boat_x=boat;g->shore_x=(int16_t)shore;g->facing=p[202]&2?-1:1;
        g->mode=p[202]&1?FISH_SHORE:FISH_MOVE;g->clock_remainder_ms=(uint16_t)p[203]*4;
    } else if(g->map_index) {
        g->boat_x=FISH_BOAT_RIGHT;g->shore_x=FISH_STATION_X;g->facing=-1;g->mode=FISH_SHORE;
    }
    g->boat_speed=g->shore_speed=0;
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
bool fish_save_unpack(fish_game_t *g,const uint8_t p[FISH_SAVE_PAYLOAD_BYTES]) {return unpack(g,p);}
void fish_save_encode(const uint8_t key[32],uint64_t seq,const uint8_t *p,uint8_t *file)
{
    memcpy(file,"FSG1",4);put16(file+4,FISH_SAVE_VERSION);put16(file+6,FISH_SAVE_PAYLOAD_BYTES);
    for(unsigned i=0;i<8;i++) file[8+i]=(uint8_t)(seq>>(8*i));
    fish_save_seal(key,file,16,p,FISH_SAVE_PAYLOAD_BYTES,file+16,file+32);
}
bool fish_save_decode(const uint8_t key[32],const uint8_t *file,size_t len,uint64_t *seq,uint8_t *p)
{
    if(len!=FISH_SAVE_LEGACY_FILE_BYTES && len!=FISH_SAVE_PREVIOUS_FILE_BYTES && len!=FISH_SAVE_V5_FILE_BYTES && len!=FISH_SAVE_V7_FILE_BYTES && len!=FISH_SAVE_V6_FILE_BYTES && len!=FISH_SAVE_V8_FILE_BYTES && len!=FISH_SAVE_V9_FILE_BYTES && len!=FISH_SAVE_V10_FILE_BYTES && len!=FISH_SAVE_V11_FILE_BYTES && len!=FISH_SAVE_FILE_BYTES) return false;
    unsigned version=get16(file+4);bool legacy=version==1;
    size_t bytes=legacy?FISH_SAVE_LEGACY_PAYLOAD_BYTES:version<5?FISH_SAVE_PREVIOUS_PAYLOAD_BYTES:version==5?FISH_SAVE_V5_PAYLOAD_BYTES:version==6?FISH_SAVE_V6_PAYLOAD_BYTES:version==7?FISH_SAVE_V7_PAYLOAD_BYTES:version==8?FISH_SAVE_V8_PAYLOAD_BYTES:version==9?FISH_SAVE_V9_PAYLOAD_BYTES:version==10?FISH_SAVE_V10_PAYLOAD_BYTES:version==11?FISH_SAVE_V11_PAYLOAD_BYTES:FISH_SAVE_PAYLOAD_BYTES;
    if(version<1 || version>FISH_SAVE_VERSION || len!=bytes+32 || memcmp(file,"FSG1",4) || get16(file+6)!=bytes) return false;
    memset(p,0,FISH_SAVE_PAYLOAD_BYTES);
    if(!fish_save_open(key,file,16,file+16,file+32,bytes,p)) return false;
    if(version<13){put32(p+12,0);p[202]=p[203]=0;}
    if(version<6)p[FISH_SAVE_V5_PAYLOAD_BYTES]=FISH_SOUND_ALL;
    if(version==2 && get32(p+200)) return false;
    if(version==3 && (p[201] || p[202] || p[203])) return false;
    if(version>=4 && p[201]!=FISH_SAVE_EQUIPMENT_EPOCH) return false;
    if(version<5 && (p[5]>=FISH_BASE_SPECIES_COUNT || (!legacy && p[187]>FISH_BASE_SPECIES_COUNT))) return false;
    if(version<5 && p[187]==FISH_BASE_SPECIES_COUNT) p[187]=FISH_SPECIES_COUNT;
    if(version>=5 && version<9 && p[187]==FISH_LEGACY_SPECIES_COUNT)p[187]=FISH_SPECIES_COUNT;
    if(version==9 && p[187]==24)p[187]=FISH_SPECIES_COUNT;
    if(version==9) {
        /* D6 silver carp used a larger cap and a different weight curve. */
        if(p[FISH_SAVE_BEST_MAP_OFFSET+18]==1)p[FISH_SAVE_BEST_MAP_OFFSET+18]=2;
        if(p[5]==18 && p[FISH_SAVE_LAST_MAP_OFFSET]==1)p[FISH_SAVE_LAST_MAP_OFFSET]=2;
        if(p[187]==18 && p[FISH_SAVE_HANGING_MAP_OFFSET]==1)p[FISH_SAVE_HANGING_MAP_OFFSET]=2;
    }
    uint64_t n=0;for(unsigned i=0;i<8;i++) n|=(uint64_t)file[8+i]<<(8*i);
    if(!n) return false;
    if(legacy) {
        /* Keep every legacy core byte/index, grant starter plus equipped items. */
        if(p[0]>=FISH_ROD_COUNT || p[1]>=FISH_LINE_COUNT || p[2]>=FISH_FLOAT_COUNT || p[3]>=FISH_SPECIAL_BAIT_COUNT) return false;
        p[64]=(uint8_t)(1u|(1u<<p[0]));p[65]=(uint8_t)(2u|(1u<<p[1]));p[66]=(uint8_t)(1u|(1u<<p[2]));
        put16(p+67+2*p[3],5);p[187]=FISH_SPECIES_COUNT;put16(p+198,360);
    }
    if(version<12)put32(p+FISH_SAVE_WEATHER_OFFSET,get16(p+198)*1000u);
    *seq=n;return true;
}
void fish_save_init(fish_save_t *s,fish_game_t *g,uint32_t now)
{
    memset(s,0,sizeof(*s));s->active_slot=-1;s->auto_at=now;s->status_at=now;
    fish_save_pack(g,s->saved);
    if(fish_storage_init(s->key)!=0) {s->status=s->blocked_status=FISH_SAVE_ERROR;s->blocked=true;s->error_code=(uint16_t)fish_storage_diagnostic();return;}
    bool invalid=false,io=false,migrated=false,equipment_initialized=false;
    unsigned best_priority=0;
    for(unsigned slot=0;slot<FISH_SAVE_SLOT_COUNT;slot++) {
        if(slot>=2 && best_priority>=10-slot/2) break;
        int result=fish_storage_read(slot,s->file,sizeof(s->file));
        size_t length=sizeof(s->file);
        if(result==-3) {length=FISH_SAVE_V11_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_V10_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_V9_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_V8_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_V7_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_V6_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_V5_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_PREVIOUS_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-3) {length=FISH_SAVE_LEGACY_FILE_BYTES;result=fish_storage_read(slot,s->file,length);}
        if(result==-2) continue;
        if(result==-1) {io=true;s->error_code=(uint16_t)fish_storage_diagnostic();continue;}
        uint64_t seq;
        if(result || !fish_save_decode(s->key,s->file,length,&seq,s->pending) ||
           !unpack(NULL,s->pending)) {invalid=true;if(!s->error_code)s->error_code=(uint16_t)(0x2000u+slot);continue;}
        equipment_initialized|=s->pending[201]==FISH_SAVE_EQUIPMENT_EPOCH;
        unsigned version=get16(s->file+4),priority=version>=5?version-4:0;
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
static bool retry_storage(fish_save_t *s)
{
    /* Re-scan storage only. Reinitializing the live game here would discard
     * catches/coins earned since the failed boot. Do not overwrite either
     * current slot until both have been read successfully. */
    uint8_t key[32];
    if(fish_storage_init(key)){s->error_code=(uint16_t)fish_storage_diagnostic();return false;}
    if(s->active_slot>=0 && memcmp(key,s->key,sizeof(key)))return false;
    if(s->active_slot<0)for(unsigned slot=2;slot<FISH_SAVE_SLOT_COUNT;slot++) {
        if(fish_storage_read(slot,s->file,sizeof(s->file))!=-2){s->error_code=0x2100;return false;}
    }
    uint64_t newest=0;int active=-1;bool invalid=false;
    for(unsigned slot=0;slot<2;slot++) {
        int result=fish_storage_read(slot,s->file,sizeof(s->file));
        if(result==-2)continue;
        if(result==-1){s->error_code=(uint16_t)fish_storage_diagnostic();return false;}
        uint64_t seq;
        if(result || !fish_save_decode(key,s->file,sizeof(s->file),&seq,s->pending) || !unpack(NULL,s->pending)) {
            invalid=true;continue;
        }
        if(active<0 || seq>newest){active=(int)slot;newest=seq;}
    }
    /* A restored card's unexamined progress must not be replaced by this
     * session's starter state. Restart once to load those existing saves. */
    if((active>=0 && (s->active_slot<0 || newest!=s->sequence)) || (active<0 && invalid)) {
        s->error_code=0x2100;return false;
    }
    memcpy(s->key,key,sizeof(key));s->active_slot=active;s->blocked=false;s->error_code=0;return true;
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
                s->error_code=0;
            } else {s->status=FISH_SAVE_ERROR;s->error_code=(uint16_t)fish_storage_diagnostic();}
        }
        return;
    }
    bool manual=s->manual_pending;s->manual_pending=false;
    if(s->status!=FISH_SAVE_IDLE && now-s->status_at>=2500) s->status=FISH_SAVE_IDLE;
    bool due=now-s->auto_at>=FISH_SAVE_INTERVAL_MS;
    if(due) s->auto_at=now;
    if(!manual && (!due || s->blocked)) return;
    if(s->blocked && !(manual && s->blocked_status==FISH_SAVE_ERROR && retry_storage(s))) {
        s->status=s->blocked_status;s->status_at=now;return;
    }
    fish_save_pack(g,s->pending);
    if(!manual && !memcmp(s->saved,s->pending,sizeof(s->saved))) return;
    if(!unpack(NULL,s->pending)) {s->status=FISH_SAVE_ERROR;s->error_code=0x2200;s->status_at=now;return;}
    if(s->sequence==UINT64_MAX) {s->status=FISH_SAVE_ERROR;s->status_at=now;return;}
    fish_save_encode(s->key,s->sequence+1,s->pending,s->file);
    s->status_at=now;s->auto_at=now;
    if(fish_storage_begin(s->active_slot==0?1:0,s->file,sizeof(s->file))) {s->status=FISH_SAVE_ERROR;s->error_code=(uint16_t)fish_storage_diagnostic();}
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
