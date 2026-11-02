#include "fish_game.h"
#include "fish_save.h"
#include "fish_save_crypto.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int fish_storage_init(uint8_t *k){(void)k;return -1;}
int fish_storage_read(unsigned s,uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -2;}
int fish_storage_begin(unsigned s,const uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -1;}
int fish_storage_poll(void){return 0;}
static void init(fish_game_t *g,unsigned day)
{
    fish_game_init(g,0);g->map_index=2;g->mode=FISH_SHORE;g->boat_x=540;g->shore_x=128;
    g->weather_ms=day*1440000u+360000;g->game_minute=360;fish_forage_update(g);
}
static void arrive(fish_game_t *g,unsigned n)
{g->mode=FISH_SHORE;g->shore_x=fish_forage_x(g,n)-16;g->facing=1;fish_forage_update(g);assert(g->forage.seen&(1u<<n));g->shore_x=fish_forage_x(g,n);}
int main(void)
{
    fish_game_t g,a;init(&g,0);g.mode=FISH_SHOP;g.shore_x=FISH_BEACH_STALL_X;g.game_minute=720;g.economy.coins=299;
    assert(fish_shop_price(FISH_SHOP_SHOVEL)==120 && fish_shop_price(FISH_SHOP_CROWBAR)==180);
    assert(fish_shop_trade(&g,FISH_SHOP_SHOVEL)==FISH_MERCHANT_BOUGHT && g.forage.tool[0]==20 && g.economy.coins==179);
    assert(fish_shop_trade(&g,FISH_SHOP_SHOVEL)==FISH_MERCHANT_OWNED && g.economy.coins==179);
    assert(fish_shop_trade(&g,FISH_SHOP_CROWBAR)==FISH_MERCHANT_POOR);
    g.economy.coins++;assert(fish_shop_trade(&g,FISH_SHOP_CROWBAR)==FISH_MERCHANT_BOUGHT && g.forage.tool[1]==20);
    g.forage.tool[0]=0;g.economy.coins=120;assert(fish_shop_trade(&g,FISH_SHOP_SHOVEL)==FISH_MERCHANT_BOUGHT && g.forage.tool[0]==20);
    bool all[FISH_SPECIES_COUNT]={0};unsigned shell=0,eligible=0,escapes=0,hits=0,worms=0,seen_days[4]={0};
    int first_lo[2]={9999,9999},first_hi[2]={0,0},last_lo[2]={9999,9999},last_hi[2]={0,0};
    bool tight_clusters[2]={false,false};
    for(unsigned day=0;day<96;day++){
        init(&g,day);for(unsigned area=0;area<2;area++){
            unsigned count=fish_forage_count(&g,area);assert(count>=3 && count<=6);seen_days[count-3]++;
            int first=fish_forage_x(&g,area*6),last=fish_forage_x(&g,area*6+count-1);
            if(first<first_lo[area])first_lo[area]=first;
            if(first>first_hi[area])first_hi[area]=first;
            if(last<last_lo[area])last_lo[area]=last;
            if(last>last_hi[area])last_hi[area]=last;
            int prior=-1000;
            for(unsigned i=0;i<6;i++){
                unsigned n=area*6+i;if(i>=count){assert(!fish_forage_exists(&g,n));continue;}
                int x=fish_forage_x(&g,n);assert(x-prior>=2 && x>=(area?288:124) && x<=(area?464:260));
                if(i && x-prior<12)tight_clusters[area]=true;
                prior=x;
                unsigned id=fish_forage_species(&g,n);all[id]=true;
                assert(area?(id==39 || id==41 || id==43 || id==FISH_GREEN_CRAB):(id==38 || id==40 || id==42 || id==FISH_RAGWORM || id==FISH_HARP_SNAIL));
                arrive(&g,n);g.forage.tool[area]=20;unsigned before=g.forage.taken;
                assert(fish_forage_start(&g) && g.forage.tool[area]==19 && g.forage.taken==(before|(1u<<n)));
                bool is_shell=id==38 || id==39 || id==40 || id==FISH_HARP_SNAIL;
                if(is_shell){shell++;assert(g.mode==FISH_CAUGHT);}
                else {eligible++;if(g.mode==FISH_FORAGE_QTE){escapes++;
                    unsigned fail_at=g.forage.at;assert(!fish_forage_hit(&g));
                    unsigned good=fail_at;for(;good<fail_at+3000;good++){g.now=good;if(fish_forage_hit(&g))break;}
                    assert(good<fail_at+3000);fish_forage_input(&g,FISH_PRESS,0);assert(g.mode==FISH_CAUGHT);hits++;
                }else assert(g.mode==FISH_CAUGHT);}
                assert(g.forage.return_shore && g.last_species==id && g.last_map==2 && g.species_catches[id]);
                assert(id==38 || id==39?(g.forage.amount>=2 && g.forage.amount<=4):id==FISH_RAGWORM?(g.forage.amount>=3 && g.forage.amount<=5):g.forage.amount==1);
                if(id==FISH_RAGWORM){worms++;assert(g.cargo_result==FISH_BAIT_STORED && g.economy.bait_stock[FISH_BAIT_RAGWORM]>0 && !g.economy.count[id] && !g.economy.cabin_count[id-25]);}
                assert(!fish_forage_exists(&g,n));
                uint8_t p[FISH_SAVE_PAYLOAD_BYTES];fish_save_pack(&g,p);fish_game_init(&a,0);assert(fish_save_unpack(&a,p));
                assert(a.forage.day==g.forage.day && a.forage.taken==g.forage.taken && a.forage.seen==g.forage.seen && a.forage.tool[area]==19);
                for(unsigned site=0;site<count;site++)assert(fish_forage_x(&a,area*6+site)==fish_forage_x(&g,area*6+site));
                a.mode=FISH_SHORE;assert(!fish_forage_exists(&a,n));
            }
        }
    }
    assert(shell && eligible>100 && escapes*100>eligible*52 && escapes*100<eligible*68 && hits==escapes && worms);
    for(unsigned i=0;i<4;i++)assert(seen_days[i]);
    for(unsigned area=0;area<2;area++)assert(tight_clusters[area] && first_hi[area]-first_lo[area]>24 && last_hi[area]-last_lo[area]>24);
    assert(all[38] && all[39] && all[40] && all[41] && all[42] && all[43] && all[FISH_RAGWORM] && all[FISH_HARP_SNAIL] && all[FISH_GREEN_CRAB]);
    assert(fish_species[FISH_HARP_SNAIL].rarity==FISH_RARITY_GOLD && fish_species[FISH_GREEN_CRAB].rarity==FISH_RARITY_GOLD);
    assert(fish_base_prices[FISH_HARP_SNAIL]==80 && fish_base_prices[FISH_GREEN_CRAB]==80);
    /* Pins follow proximity in both directions, including across area edges.
     * Hiding does not forget discovery or restore harvested spots. */
    for(unsigned node=0;node<=6;node+=6){
        init(&g,0);g.forage.seen=0;int x=fish_forage_x(&g,node);
        g.shore_x=x-49;g.facing=-1;fish_forage_update(&g);assert(!fish_forage_visible(&g,node));
        g.shore_x=x-48;fish_forage_update(&g);assert(fish_forage_visible(&g,node));
        g.shore_x=x+49;fish_forage_update(&g);assert(!fish_forage_visible(&g,node) && (g.forage.seen&(1u<<node)));
        g.shore_x=x+48;fish_forage_update(&g);assert(fish_forage_visible(&g,node));
        g.forage.taken|=1u<<node;assert(!fish_forage_visible(&g,node));
    }
    /* Walking crosses revealed traces freely, in both directions and gears. */
    init(&g,0);g.shore_x=fish_forage_x(&g,0)-4;g.facing=1;g.shore_speed=1;fish_forage_update(&g);
    int start_x=g.shore_x;fish_game_update(&g,250);assert(g.shore_x==start_x+8 && g.shore_speed==1);
    g.facing=-1;g.shore_speed=-2;start_x=g.shore_x;fish_game_update(&g,500);
    assert(g.shore_x==start_x-16 && g.shore_speed==-2);
    /* When reach circles overlap, a click selects the closest trace. */
    bool overlap=false;
    for(unsigned day=0;day<96 && !overlap;day++){
        init(&g,day);g.forage.seen=4095;
        for(unsigned n=1;n<fish_forage_count(&g,0);n++){
            int gap=fish_forage_x(&g,n)-fish_forage_x(&g,n-1);
            if(gap>22)continue;
            g.shore_x=fish_forage_x(&g,n)-(gap/2-1);
            assert(fish_forage_near(&g)==(int)n);overlap=true;break;
        }
    }assert(overlap);
    /* Real encoder click consumes one tool use, and the result returns ashore. */
    init(&g,0);unsigned direct=0;
    while(direct+1<fish_forage_count(&g,0) && fish_forage_species(&g,direct)!=38)direct++;
    arrive(&g,direct);g.forage.tool[0]=20;
    fish_game_input(&g,FISH_PRESS,0,1);fish_game_input(&g,FISH_RELEASE,0,30);
    if(g.mode==FISH_FORAGE_QTE){g.now=g.forage.at+200;while(!fish_forage_hit(&g))g.now++;fish_forage_input(&g,FISH_PRESS,0);}
    assert(g.mode==FISH_CAUGHT && g.forage.tool[0]==19);
    fish_game_update(&g,g.phase_at+4000);assert(g.mode==FISH_SHORE && !g.forage.return_shore);
    /* A full bait box cannot destroy a trace or charge durability. */
    bool full=false;
    for(unsigned day=0;day<96 && !full;day++){init(&g,day);for(unsigned n=0;n<fish_forage_count(&g,0);n++)if(fish_forage_species(&g,n)==FISH_RAGWORM){
        arrive(&g,n);g.forage.tool[0]=20;g.economy.bait_stock[FISH_BAIT_RAGWORM]=FISH_BAIT_STOCK_LIMIT;
        unsigned taken=g.forage.taken;assert(fish_forage_start(&g) && g.forage.reply==FISH_FORAGE_WORM_FULL && g.forage.tool[0]==20 && g.forage.taken==taken);full=true;break;
    }}assert(full);
    /* A batch may partially fit; only accepted individuals enter the ledger. */
    bool partial=false;
    for(unsigned day=0;day<96 && !partial;day++){init(&g,day);for(unsigned n=0;n<fish_forage_count(&g,0);n++)if(fish_forage_species(&g,n)==38){
        arrive(&g,n);g.forage.tool[0]=20;a=g;assert(fish_forage_start(&a) && a.mode==FISH_CAUGHT);
        unsigned weight=a.last_weight_g;
        g.economy.count[0]=1;g.economy.grams[0]=fish_hold_capacity(&g.economy)-weight;g.economy.value[0]=17;
        assert(fish_forage_start(&g) && g.mode==FISH_CAUGHT && g.forage.amount>=2 && g.forage.stored==1 && g.forage.tool[0]==19);
        assert(g.economy.count[38]==1 && g.economy.grams[38]==weight && fish_hold_weight(&g.economy)==fish_hold_capacity(&g.economy));
        assert(g.cargo_result==FISH_STORED && g.species_catches[38]==g.forage.amount && g.economy.value[0]==17);partial=true;break;
    }}assert(partial);
    bool partial_bait=false;
    for(unsigned day=0;day<96 && !partial_bait;day++){init(&g,day);for(unsigned n=0;n<fish_forage_count(&g,0);n++)if(fish_forage_species(&g,n)==FISH_RAGWORM){
        arrive(&g,n);g.forage.tool[0]=20;g.economy.bait_stock[FISH_BAIT_RAGWORM]=998;assert(fish_forage_start(&g));
        if(g.mode==FISH_FORAGE_QTE){while(!fish_forage_hit(&g))g.now++;fish_forage_input(&g,FISH_PRESS,0);}
        assert(g.mode==FISH_CAUGHT && g.forage.amount>=3 && g.forage.stored==1 && g.economy.bait_stock[FISH_BAIT_RAGWORM]==999 && g.forage.tool[0]==19);
        assert(!fish_forage_exists(&g,n));partial_bait=true;break;
    }}assert(partial_bait);
    /* Fresh bait is consumed on casting, and the empty stock falls back safely. */
    uint64_t waits[2]={0};unsigned bites[2]={0};
    for(unsigned bait=0;bait<2;bait++)for(unsigned seed=1;seed<=80;seed++){
        fish_game_init(&g,0);g.map_index=2;g.boat_x=760;g.facing=1;g.game_minute=720;
        g.random_state=seed*0x9e3779b9u;g.bait_index=bait?FISH_BAIT_RAGWORM:FISH_PLASTIC_BAIT;g.depth_index=0;
        g.economy.bait_stock[FISH_BAIT_RAGWORM]=1;
        fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,1000);
        assert(g.mode==FISH_CAST && g.economy.bait_stock[FISH_BAIT_RAGWORM]==(bait?0:1));
        fish_game_update(&g,3000);
        if(g.pending_species<FISH_SPECIES_COUNT){waits[bait]+=g.bite_wait_ms;bites[bait]++;}
        if(bait){g.mode=FISH_AIM;fish_game_input(&g,FISH_PRESS,0,3001);fish_game_input(&g,FISH_RELEASE,0,4001);assert(g.bait_index==FISH_PLASTIC_BAIT);}
    }
    printf("Fresh bait wait/bites: plastic=%llu/%u ragworm=%llu/%u\n",(unsigned long long)waits[0],bites[0],(unsigned long long)waits[1],bites[1]);
    assert(bites[0] && bites[1] && waits[1]*bites[0]*100<waits[0]*bites[1]*80);
    /* Invalid appended state is rejected before touching the running game. */
    init(&g,2);uint8_t corrupt[FISH_SAVE_PAYLOAD_BYTES];fish_save_pack(&g,corrupt);a=g;
    corrupt[FISH_SAVE_FORAGE_OFFSET]=21;assert(!fish_save_unpack(&g,corrupt) && !memcmp(&g,&a,sizeof(g)));
    init(&g,2);arrive(&g,0);unsigned mask=g.forage.taken;assert(fish_forage_start(&g) && g.forage.reply==FISH_FORAGE_NEED_SHOVEL && g.forage.taken==mask);
    g.forage.taken=4095;g.forage.seen=4095;g.weather_ms=3*1440000u+239000;g.forage.day=fish_forage_day(&g);fish_forage_update(&g);assert(g.forage.taken==4095);
    g.weather_ms+=1000;fish_forage_update(&g);assert(!g.forage.taken && g.forage.seen!=4095);
    /* A failed QTE and a timeout each clear exactly one point and one tool use. */
    bool failed=false;
    for(unsigned day=0;day<96 && !failed;day++){init(&g,day);for(unsigned n=0;n<12;n++)if(fish_forage_exists(&g,n)){
        arrive(&g,n);g.forage.tool[n/6]=20;fish_forage_start(&g);if(g.mode!=FISH_FORAGE_QTE)continue;
        a=g;fish_forage_input(&g,FISH_PRESS,0);assert(g.mode==FISH_SHORE && g.forage.reply==FISH_FORAGE_ESCAPED && g.forage.tool[n/6]==19 && !fish_forage_exists(&g,n));
        a.now=a.forage.at+FISH_FORAGE_QTE_MS;fish_forage_update(&a);assert(a.mode==FISH_SHORE && a.forage.reply==FISH_FORAGE_ESCAPED && a.forage.tool[n/6]==19);failed=true;break;
    }}assert(failed);
    /* Every moving species keeps a continuous press window of at least200ms. */
    const unsigned mobile[5]={41,42,43,FISH_RAGWORM,FISH_GREEN_CRAB};
    for(unsigned which=0;which<5;which++){
        bool checked=false;
        for(unsigned day=0;day<96 && !checked;day++){
            init(&g,day);
            for(unsigned n=0;n<12;n++)if(fish_forage_exists(&g,n) && fish_forage_species(&g,n)==mobile[which]){
                g.forage.node=n;g.forage.at=0;unsigned run=0,best=0;
                for(unsigned t=0;t<2000;t++){g.now=t;run=fish_forage_hit(&g)?run+1:0;if(run>best)best=run;}
                assert(best>=200);checked=true;break;
            }
        }assert(checked);
    }
    /* Authentic D24 V19 save migrates without interpreting empty stern47 as a worm. */
    uint8_t key[32]={7},payload[FISH_SAVE_PAYLOAD_BYTES],out[FISH_SAVE_PAYLOAD_BYTES],file[FISH_SAVE_V19_FILE_BYTES]={0};uint64_t seq;
    fish_game_init(&g,0);g.economy.coins=12345;fish_save_pack(&g,payload);payload[187]=47;
    memcpy(file,"FSG1",4);file[4]=19;file[6]=FISH_SAVE_V19_PAYLOAD_BYTES&255;file[7]=FISH_SAVE_V19_PAYLOAD_BYTES>>8;file[8]=42;
    fish_save_seal(key,file,16,payload,FISH_SAVE_V19_PAYLOAD_BYTES,file+16,file+32);
    assert(fish_save_decode(key,file,sizeof(file),&seq,out));fish_game_init(&a,0);assert(fish_save_unpack(&a,out) && a.economy.coins==12345 && a.economy.hanging_species==FISH_SPECIES_COUNT && !a.forage.tool[0] && !a.economy.bait_stock[FISH_BAIT_RAGWORM]);
    /* V20's old forage tail must move after the two appended25-byte records. */
    init(&g,7);g.economy.coins=54321;g.forage.tool[0]=17;g.forage.tool[1]=9;g.forage.taken=5;g.forage.seen=37;g.economy.bait_stock[FISH_BAIT_RAGWORM]=11;
    g.economy.count[0]=1;g.economy.grams[0]=100;g.economy.value[0]=17;g.economy.prices[0]=(fish_price_group_t){17,1};
    fish_save_pack(&g,payload);uint8_t old20[FISH_SAVE_V20_PAYLOAD_BYTES],file20[FISH_SAVE_V20_FILE_BYTES]={0};
    memcpy(old20,payload,FISH_SAVE_V20_FORAGE_OFFSET);memcpy(old20+FISH_SAVE_V20_FORAGE_OFFSET,payload+FISH_SAVE_FORAGE_OFFSET,10);old20[187]=48;
    memcpy(file20,"FSG1",4);file20[4]=20;file20[6]=sizeof(old20)&255;file20[7]=sizeof(old20)>>8;file20[8]=43;
    fish_save_seal(key,file20,16,old20,sizeof(old20),file20+16,file20+32);
    assert(fish_save_decode(key,file20,sizeof(file20),&seq,out));fish_game_init(&a,0);assert(fish_save_unpack(&a,out));
    assert(a.economy.coins==54321 && a.forage.tool[0]==17 && a.forage.tool[1]==9 && a.forage.taken==5 && a.forage.seen==37 && a.economy.bait_stock[FISH_BAIT_RAGWORM]==11);
    assert(a.economy.hanging_species==FISH_SPECIES_COUNT && a.economy.value[0]==17 && !a.species_catches[FISH_HARP_SNAIL] && !a.species_catches[FISH_GREEN_CRAB]);
    /* Three-game-month economic audit, with0% and75% success on escape QTEs. */
    for(unsigned skill=0;skill<=75;skill+=75){
        long long net=0;unsigned gross=0,cost=0,bait=0,gold=0;
        for(unsigned day=0;day<96;day++){
            init(&g,day);g.forage.tool[0]=g.forage.tool[1]=20;
            for(unsigned n=0;n<12;n++)if(fish_forage_exists(&g,n)){
                arrive(&g,n);cost+=n<6?6:9;fish_forage_start(&g);
                if(g.mode==FISH_FORAGE_QTE){
                    if(skill && (day*17+n*13)%4){g.now=g.forage.at;while(!fish_forage_hit(&g))g.now++;}
                    fish_forage_input(&g,FISH_PRESS,0);
                }
                if(g.mode==FISH_CAUGHT && (g.last_species==FISH_HARP_SNAIL || g.last_species==FISH_GREEN_CRAB))gold++;
            }
            gross+=fish_cargo_total_value(&g);bait+=g.economy.bait_stock[FISH_BAIT_RAGWORM];
        }
        net=(long long)gross-cost;assert(net>0);
        printf("FORAGE_ECONOMY,%u,96,%u,%u,%lld,%u,%u\n",skill,gross,cost,net,bait,gold);
    }
    printf("PASS:96 daily schedules,3-6 points/area,9species, single20-use tools, shells=%u; escapeQTE=%u/%u; successfulQTE=%u,ragworms=%u; failed/timeout,V20 state,V19 migration\n",shell,escapes,eligible,hits,worms);
}
