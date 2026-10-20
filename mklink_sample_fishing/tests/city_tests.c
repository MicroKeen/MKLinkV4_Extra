#include "fish_game.h"
#include "fish_save.h"
#include "fish_save_crypto.h"
#include "fish_save_storage.h"
#include "fish_merchant_dialogue_data.h"
#include "fish_city_monger_dialogue_data.h"
#define fish_city_dialogue fish_city_monger_dialogue
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t legacy[FISH_SAVE_V8_FILE_BYTES],current[2][FISH_SAVE_FILE_BYTES];
static uint8_t legacy9[FISH_SAVE_V9_FILE_BYTES];
static bool v9_present;
static bool old_present,present[2];
static const uint8_t key[32]={7,9,2};
int fish_storage_init(uint8_t *out){memcpy(out,key,32);return 0;}
int fish_storage_read(unsigned slot,uint8_t *out,size_t n){
    assert(slot<FISH_SAVE_SLOT_COUNT);
    if(slot<2 && present[slot]){if(n!=sizeof(current[slot]))return -3;memcpy(out,current[slot],n);return 0;}
    if(slot==2 && v9_present){if(n!=sizeof(legacy9))return -3;memcpy(out,legacy9,n);return 0;}
    if(slot==4 && old_present){if(n!=sizeof(legacy))return -3;memcpy(out,legacy,n);return 0;}
    return -2;
}
int fish_storage_begin(unsigned slot,const uint8_t *data,size_t n){assert(slot<2 && n==sizeof(current[slot]));memcpy(current[slot],data,n);present[slot]=true;return 0;}
int fish_storage_poll(void){return 1;}
static void cast(fish_game_t *g,uint32_t seed,unsigned bait,unsigned depth,unsigned boat,int facing){
    fish_game_init(g,0);g->map_index=1;g->game_minute=720;g->boat_x=(int)boat;g->facing=facing;
    g->rod_index=2;g->bait_index=(uint8_t)bait;g->depth_index=(uint8_t)depth;g->random_state=seed;
    if(bait<5)g->economy.bait_stock[bait]=1;
    fish_game_input(g,FISH_PRESS,0,0);fish_game_input(g,FISH_RELEASE,0,1500);assert(g->mode==FISH_CAST);
}
int main(void){
    const uint8_t ids[]={0,1,15,7,16,17,18,19,20,24,22,23};
    const uint16_t lengths[]={450,1300,1100,250,1450,1700,1000,750,2000,1450,1000,400};
    const uint32_t grams[]={2500,40000,12000,150,45000,70000,40000,6000,55000,60000,15000,3000};
    for(unsigned i=0;i<12;i++){
        const fish_species_t *p=fish_species_profile(1,ids[i]);assert(p->max_mm==lengths[i]);
        assert(fish_species_weight_g(p,p->max_mm)==grams[i]);
        assert(fish_catch_price_on(1,ids[i],p->max_mm)>0);
    }
    assert(fish_species[0].max_mm==400 && fish_species[1].max_mm==1000 && fish_species[1].rarity==0);
    assert(fish_species_profile(1,1)->rarity==FISH_RARITY_SILVER && fish_species[17].rarity==FISH_RARITY_SILVER);
    assert(fish_species[18].rarity==FISH_RARITY_BRONZE && fish_species[24].rarity==FISH_RARITY_SILVER);
    assert(fish_map_depth_on(1,96)==360 && fish_map_depth_on(1,864)==420 && fish_map_depth_on(1,480)==1600);
    assert(fish_map_depth(96)==60 && fish_map_depth(480)==600);
    for(unsigned x=57;x<904;x++) {
        int bottom=fish_map_depth_on(1,x);
        assert(bottom>=300 && bottom<=1600);
        for(unsigned bait=0;bait<6;bait++) {
            assert(!fish_map_weight_on(1,21,x,0,bait,720));
            assert(!fish_map_weight_on(1,21,x,bottom,bait,720));
            assert(!fish_map_weight_on(1,18,x,bottom/2,bait,720));
            assert(!fish_map_weight_on(1,24,x,0,bait,720));
            assert(!fish_map_weight_on(1,24,x,bottom,bait,720));
        }
    }
    assert(fish_map_weight_on(1,18,480,0,2,720)>0);
    assert(fish_map_weight_on(1,24,480,900,2,720)>0 && fish_map_weight_on(1,24,480,1200,2,720)>0);
    unsigned guide_seen=0;
    for(unsigned p=0;p<FISH_GUIDE_COUNT;p++) {
        unsigned id=fish_guide_index_at(p);assert(id!=21 && fish_guide_position(id)==p);guide_seen|=1u<<id;
    }
    assert(guide_seen&(1u<<24));
    for(unsigned id=15;id<FISH_SPECIES_COUNT;id++){
        uint32_t state=42;assert(fish_species_parent(id)==id && !fish_species_is_variant(id));
        assert(fish_species_special_roll(&state,id)==id && state==42);
        for(unsigned bait=0;bait<6;bait++)assert(fish_map_weight_on(0,id,480,600,bait,720)==0);
    }
    for(unsigned size=150;size<=1300;size+=10)assert(fish_catch_price_on(1,13,size)==10*fish_catch_price_on(1,1,size));
    unsigned seen[FISH_SPECIES_COUNT]={0};fish_game_t g,loaded;
    for(unsigned trial=1;trial<=60000;trial++){
        cast(&g,trial*0x9e3779b9u,trial%6,(unsigned[]){0,5,7}[(trial/6)%3],(unsigned[]){132,280,480,700,828}[(trial/18)%5],trial&1?1:-1);
        assert(g.cast_bottom_cm==fish_map_depth_on(1,g.cast_x));
        assert(g.pending_species<FISH_SPECIES_COUNT && g.pending_species!=21 && fish_map_weight_on(1,g.pending_species,g.cast_x,g.cast_depth_cm,g.bait_index,720)>0);
        const fish_species_t *p=fish_species_profile(1,g.pending_species);
        assert(g.pending_size_mm>=p->min_mm && g.pending_size_mm<=p->max_mm);
        assert(g.pending_weight_g==fish_species_weight_g(p,g.pending_size_mm));++seen[g.pending_species];
        if(g.pending_variant)assert(g.pending_species==1 && g.pending_variant==13);
    }
    for(unsigned i=0;i<12;i++)assert(seen[ids[i]]>10);
    assert(fish_map_weight_on(1,20,480,0,3,720)>0 && !fish_map_weight_on(1,20,480,0,1,720));
    for(unsigned map=0;map<2;map++){
        fish_game_init(&g,0);g.map_index=(uint8_t)map;
        unsigned open=map?360:420,close=map?960:1080;
        g.game_minute=open-1;assert(!fish_game_shop_open(&g));g.game_minute=open;assert(fish_game_shop_open(&g));
        g.game_minute=close-1;assert(fish_game_shop_open(&g));g.game_minute=close;assert(!fish_game_shop_open(&g));
    }
    static const fish_merchant_reply_t replies[]={FISH_MERCHANT_SOLD,FISH_MERCHANT_BOUGHT,FISH_MERCHANT_POOR,
        FISH_MERCHANT_EMPTY,FISH_MERCHANT_OWNED,FISH_MERCHANT_FULL};
    unsigned poses=0;
    for(unsigned context=0;context<14;context++) {
        fish_game_init(&g,0);g.map_index=1;g.merchant_reply=FISH_MERCHANT_GREETING;
        if(context==1)fish_hold_add(&g.economy,0,150,100);
        else if(context==2)fish_hold_add(&g.economy,1,1000,20000);
        else if(context>=3 && context<=8)g.merchant_reply=replies[context-3];
        else if(context>=9 && context<=12)fish_hold_add(&g.economy,11+context-9,300,100);
        else if(context==13)g.merchant_hint=true;
        for(unsigned line=0;line<2;line++) {
            g.merchant_line_variant=line;
            assert(fish_merchant_pose(&g)==fish_city_dialogue[context].pose);
            assert(!strcmp(fish_merchant_says(&g),fish_city_dialogue[context].lines[line]));
            poses|=1u<<fish_merchant_pose(&g);
        }
    }
    assert(poses==15);
    fish_game_init(&g,0);g.last_species=1;g.last_map=1;g.last_size_mm=1300;g.catches=1;
    g.species_catches[1]=1;g.species_best_size_mm[1]=1300;g.species_best_map[1]=1;
    assert(fish_hold_add_on(&g.economy,1,1,1300,40000)==FISH_HANGING);
    uint8_t payload[FISH_SAVE_PAYLOAD_BYTES],file[FISH_SAVE_FILE_BYTES],decoded[FISH_SAVE_PAYLOAD_BYTES];uint64_t sequence;
    fish_save_pack(&g,payload);fish_save_encode(key,3,payload,file);
    assert(fish_save_decode(key,file,sizeof(file),&sequence,decoded));fish_game_init(&loaded,0);
    assert(fish_save_unpack(&loaded,decoded) && loaded.map_index==0 && loaded.last_map==1 && loaded.last_weight_g==40000);
    assert(loaded.species_best_map[1]==1 && loaded.economy.hanging_map==1);
    loaded.map_index=1;loaded.game_minute=720;uint32_t value=fish_hold_value(&loaded.economy);
    assert(fish_shop_trade(&loaded,0)==FISH_MERCHANT_SOLD && loaded.economy.coins==value);
    /* Authenticate an actual V8 prefix and migrate without touching its bytes. */
    fish_game_init(&g,0);g.map_index=1;g.rod_index=2;g.economy.rods=7;g.economy.coins=321;
    g.last_species=1;g.last_size_mm=450;g.catches=1;g.species_catches[1]=1;g.species_best_size_mm[1]=450;
    fish_save_pack(&g,payload);payload[187]=15;
    memcpy(legacy,"FSG1",4);legacy[4]=8;legacy[6]=(uint8_t)FISH_SAVE_V8_PAYLOAD_BYTES;legacy[7]=FISH_SAVE_V8_PAYLOAD_BYTES>>8;legacy[8]=17;
    fish_save_seal(key,legacy,16,payload,FISH_SAVE_V8_PAYLOAD_BYTES,legacy+16,legacy+32);
    uint8_t original[sizeof(legacy)];memcpy(original,legacy,sizeof(legacy));old_present=true;
    fish_save_t save;fish_game_init(&loaded,0);fish_save_init(&save,&loaded,0);
    assert(!save.blocked && loaded.save_requested && save.sequence==17 && loaded.map_index==1 && loaded.rod_index==2 && loaded.economy.coins==321);
    assert(loaded.species_catches[1]==1 && loaded.species_best_map[1]==0 && loaded.economy.hanging_species==FISH_SPECIES_COUNT);
    fish_save_tick(&save,&loaded,1);fish_save_tick(&save,&loaded,2);
    assert(save.sequence==18 && !memcmp(legacy,original,sizeof(legacy)));
    fish_game_init(&loaded,0);fish_save_init(&save,&loaded,0);assert(save.sequence==18 && !loaded.save_requested);
    /* Old redtail is not reinterpreted as bighead; oversized D6 silver carp
     * retains its old weight and price through V9 -> V10 -> restart. */
    memset(present,0,sizeof(present));old_present=false;v9_present=true;
    fish_game_init(&g,0);g.map_index=1;g.last_species=18;g.last_map=1;g.last_size_mm=1300;g.catches=2;
    g.species_catches[18]=1;g.species_best_size_mm[18]=1300;g.species_best_map[18]=1;
    g.species_catches[21]=1;g.species_best_size_mm[21]=400;g.species_best_map[21]=1;
    assert(fish_hold_add_on(&g.economy,2,18,1300,40000)==FISH_HANGING);
    uint32_t old_price=g.economy.hanging_value;g.economy.hanging_map=1;
    fish_save_pack(&g,payload);memset(legacy9,0,sizeof(legacy9));memcpy(legacy9,"FSG1",4);
    legacy9[4]=9;legacy9[6]=(uint8_t)FISH_SAVE_V9_PAYLOAD_BYTES;legacy9[7]=FISH_SAVE_V9_PAYLOAD_BYTES>>8;legacy9[8]=30;
    fish_save_seal(key,legacy9,16,payload,FISH_SAVE_V9_PAYLOAD_BYTES,legacy9+16,legacy9+32);
    uint8_t original9[sizeof(legacy9)];memcpy(original9,legacy9,sizeof(legacy9));
    fish_game_init(&loaded,0);fish_save_init(&save,&loaded,0);
    assert(!save.blocked && loaded.save_requested && loaded.species_catches[21]==1 && loaded.species_catches[24]==0);
    assert(loaded.last_weight_g==40000 && loaded.last_map==2 && loaded.species_best_map[18]==2);
    assert(loaded.economy.hanging_value==old_price && loaded.economy.hanging_grams==40000 && loaded.economy.hanging_map==2);
    fish_save_tick(&save,&loaded,1);fish_save_tick(&save,&loaded,2);
    assert(!memcmp(legacy9,original9,sizeof(legacy9)));
    fish_game_init(&loaded,0);fish_save_init(&save,&loaded,0);assert(!save.blocked && save.sequence==31);
    assert(loaded.last_weight_g==40000 && loaded.species_catches[24]==0);
    loaded.game_minute=720;
    assert(fish_shop_trade(&loaded,0)==FISH_MERCHANT_SOLD && loaded.economy.coins==old_price);
    fish_game_init(&g,0);g.map_index=1;g.last_species=24;g.last_map=1;g.last_size_mm=1450;g.catches=1;
    g.species_catches[24]=1;g.species_best_size_mm[24]=1450;g.species_best_map[24]=1;
    assert(fish_hold_add_on(&g.economy,1,24,1450,60000)==FISH_HANGING);
    fish_save_pack(&g,payload);fish_save_encode(key,40,payload,file);
    assert(fish_save_decode(key,file,sizeof(file),&sequence,decoded));fish_game_init(&loaded,0);
    assert(fish_save_unpack(&loaded,decoded) && loaded.last_weight_g==60000 && loaded.economy.hanging_species==24);
    puts("PASS: CSV city limits, all twelve actual cast outcomes, bait/layer rules, 09-21 shop, four clerk expressions and 28 lines, cross-map giant price/save origins, immutable V8 migration to V9");
}
