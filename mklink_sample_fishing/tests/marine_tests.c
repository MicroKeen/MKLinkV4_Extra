#include "fish_game.h"
#include "fish_save.h"
#include "fish_save_crypto.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int fish_storage_init(uint8_t *k){(void)k;return -1;}
int fish_storage_read(unsigned i,uint8_t *p,size_t n){(void)i;(void)p;(void)n;return -2;}
int fish_storage_begin(unsigned i,const uint8_t *p,size_t n){(void)i;(void)p;(void)n;return -1;}
int fish_storage_poll(void){return 0;}
static void hook(fish_game_t *g,unsigned grams,unsigned seed)
{
    fish_game_init(g,0);g->map_index=2;g->rod_index=FISH_LONG_ROD;g->economy.rods|=1u<<FISH_LONG_ROD;fish_game_equip_line(g,FISH_PE_LINE);
    g->mode=FISH_BITE;g->pending_species=37;g->pending_size_mm=2000;g->pending_weight_g=grams;g->random_state=seed;
    g->phase_at=0-fish_species[37].strike_delay_ms;g->cast_start_x=1260;g->cast_distance=fish_game_max_cast_px(g);g->cast_x=1400;
    fish_game_input(g,FISH_PRESS,0,0);fish_game_input(g,FISH_RELEASE,0,0);assert(g->mode==FISH_FIGHT);
}
static unsigned trial(unsigned grams,unsigned seed,unsigned *peak,unsigned *duration)
{
    fish_game_t g;hook(&g,grams,seed);*peak=0;
    for(unsigned t=100;t<=180000 && g.mode==FISH_FIGHT;t+=100){
        fish_game_update(&g,t);if(g.tension>(int)*peak)*peak=g.tension;
        int steps=g.tension>=g.line_strength*65/100?-1:(fish_game_burst_active(&g)||fish_game_burst_warning(&g)) && g.tension>=g.line_strength/2?0:1;
        fish_game_input(&g,FISH_ROTATE,steps,t);
    }
    *duration=g.now;return g.mode==FISH_CAUGHT;
}
int main(void)
{
    setvbuf(stdout,NULL,_IONBF,0);
    bool found[FISH_SPECIES_COUNT]={0};unsigned tiers[4]={0};
    for(unsigned id=25;id<FISH_SPECIES_COUNT;id++){
        const fish_species_t *s=&fish_species[id];uint32_t prior=0;
        for(unsigned mm=s->min_mm;mm<=s->max_mm;mm++){uint32_t w=fish_species_weight_g(s,mm);assert(w>=prior);prior=w;}
    }
    assert(fish_species[28].max_mm==2000 && fish_species_weight_g(&fish_species[28],2000)==50000);
    assert(fish_species_weight_g(&fish_species[37],2800)==350000);
    for(unsigned x=540;x<=1400;x+=97)for(unsigned layer=0;layer<3;layer++)for(unsigned bait=3;bait<=8;bait++) {
        int bottom=fish_map_depth_on(2,x),hook=layer==0?0:layer==1?bottom/2:bottom;
        for(unsigned id=0;id<FISH_SPECIES_COUNT;id++)if(fish_map_weight_on(2,id,x,hook,bait,720)){
            assert(id>=25 && !fish_species_is_foraging(id) && id!=FISH_OARFISH && id!=FISH_PEACOCK);found[id]=true;
        }
        for(unsigned seed=1;seed<=300;seed++) {
            uint32_t random=seed*0x9e3779b9u;uint8_t id;uint16_t mm;
            if(fish_map_roll_on(&random,2,x,hook,bait,720,&id,&mm))++tiers[fish_species[id].rarity];
        }
    }
    for(unsigned id=25;id<=37;id++)if(id!=FISH_OARFISH)assert(found[id]);
    assert(found[FISH_SQUID]);
    unsigned total=tiers[0]+tiers[1]+tiers[2];assert(tiers[0]>total/2 && tiers[2]>0 && tiers[2]<total/10);
    unsigned rare[4]={0};
    for(unsigned n=1;n<=100000;n++)for(unsigned cat=0;cat<2;cat++) {
        uint32_t random=n*0x9e3779b9u;uint8_t id;uint16_t mm;
        rare[cat]+=fish_map_coast_special(&random,1320,5250,4,255,255,cat,&id,&mm);
        random=n*0x9e3779b9u;rare[cat+2]+=(cat?fish_species_special_roll_cat(&random,25):fish_species_special_roll(&random,25))==FISH_PEACOCK;
    }
    assert(rare[0]>850 && rare[0]<1150 && rare[1]>2250 && rare[1]<2750);
    assert(rare[2]>850 && rare[2]<1150 && rare[3]>2250 && rare[3]<2750);
    uint32_t random=7;uint8_t id=0;uint16_t mm=0;
    assert(!fish_map_coast_special(&random,1320,5250,4,255,128,false,&id,&mm) && random==7);
    assert(!fish_map_coast_special(&random,1000,600,4,255,255,false,&id,&mm));
    fish_game_t g,a;fish_game_init(&g,0);g.map_index=2;g.mode=FISH_SHOP;g.game_minute=720;g.economy.coins=10000;
    assert(fish_shop_trade(&g,FISH_SHOP_CABIN)==FISH_MERCHANT_BOUGHT && g.economy.cabin_owned);
    assert(fish_hold_add_on(&g.economy,2,32,1500,100000)==FISH_CABIN_STORED);
    assert(fish_hold_add_on(&g.economy,2,37,1800,100000)==FISH_CABIN_STORED);
    assert(fish_cabin_weight(&g.economy)==200000);
    assert(fish_hold_add_on(&g.economy,2,32,1600,100001)!=FISH_CABIN_STORED && fish_cabin_weight(&g.economy)==200000);
    g.economy.hanging_species=FISH_SPECIES_COUNT;g.economy.hanging_grams=g.economy.hanging_value=g.economy.hanging_size_mm=0;
    fish_hold_add_on(&g.economy,0,0,200,100);unsigned cabin=fish_cabin_value(&g.economy),coins=g.economy.coins;
    g.map_index=1;assert(fish_shop_trade(&g,0)==FISH_MERCHANT_SOLD && g.economy.coins>coins && fish_cabin_value(&g.economy)==cabin);
    uint8_t payload[FISH_SAVE_PAYLOAD_BYTES];fish_save_pack(&g,payload);fish_game_init(&a,0);assert(fish_save_unpack(&a,payload) && fish_cabin_weight(&a.economy)==200000 && a.economy.cabin_owned);
    a.map_index=2;a.mode=FISH_SHOP;a.game_minute=720;coins=a.economy.coins;assert(fish_shop_trade(&a,0)==FISH_MERCHANT_SOLD && a.economy.coins==coins+cabin && !fish_cabin_weight(&a.economy));
    fish_game_init(&g,0);g.economy.coins=12345;g.rod_index=2;g.economy.rods=7;fish_save_pack(&g,payload);payload[187]=25;
    uint8_t old[FISH_SAVE_V17_FILE_BYTES]={0},key[32]={1},out[FISH_SAVE_PAYLOAD_BYTES];uint64_t seq;
    memcpy(old,"FSG1",4);old[4]=17;old[6]=FISH_SAVE_V17_PAYLOAD_BYTES&255;old[7]=FISH_SAVE_V17_PAYLOAD_BYTES>>8;old[8]=9;
    fish_save_seal(key,old,16,payload,FISH_SAVE_V17_PAYLOAD_BYTES,old+16,old+32);
    assert(fish_save_decode(key,old,sizeof(old),&seq,out));fish_game_init(&a,0);assert(fish_save_unpack(&a,out) && a.economy.coins==12345 && a.rod_index==2 && a.economy.hanging_species==FISH_SPECIES_COUNT);
    /* No final-distance or stamina gate: exhausted marine fish alongside
     * the boat land immediately, including the largest bluefin. */
    for(unsigned i=25;i<FISH_SPECIES_COUNT;i++)if(!fish_species_is_foraging(i)){
        hook(&g,fish_species_weight_g(&fish_species[i],fish_species[i].max_mm),42);
        g.pending_species=i;g.pending_size_mm=fish_species[i].max_mm;
        g.reel_units=9999;g.tension=0;g.stamina_recovering=true;
        fish_game_input(&g,FISH_ROTATE,1,0);assert(g.mode==FISH_CAUGHT && g.reel_units==10000);
    }
    hook(&g,350000,42);g.reel_units=9500;g.tension=0;
    fish_game_input(&g,FISH_ROTATE,1,0);assert(g.reel_units>9500);
    puts("weight_g,landed_of16,mean_duration_ms,mean_peak");
    unsigned catches[3];
    for(unsigned grams=100000;grams<=300000;grams+=100000){unsigned landed=0;uint64_t times=0,peaks=0;
        for(unsigned seed=1;seed<=16;seed++){unsigned peak,duration;landed+=trial(grams,seed,&peak,&duration);times+=duration;peaks+=peak;}
        printf("%u,%u,%llu,%llu\n",grams,landed,(unsigned long long)(times/16),(unsigned long long)(peaks/16));
        catches[grams/100000-1]=landed;
    }
    /* An old river PE rig can still land100kg; larger pelagic loads are risky.
     * This fixed16-seed controller sample is not a human success-rate target. */
    assert(catches[0]>0 && catches[0]>catches[1] && catches[0]>catches[2] && catches[2]<8);
    printf("PASS:46 species;bronze/silver/gold=%u/%u/%u; oarfish=%u/%u,peacock=%u/%u per100000; isolated200kg cabin,V17 migration\n",tiers[0],tiers[1],tiers[2],rare[0],rare[1],rare[2],rare[3]);
}
