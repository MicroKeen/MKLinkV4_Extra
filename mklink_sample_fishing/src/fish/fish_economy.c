#include "fish_economy.h"
#include "fish_game.h"
#include <string.h>
#include "fish_merchant_dialogue_data.h"
#include "fish_city_dialogue_data.h"
#include "fish_city_monger_dialogue_data.h"
#include "fish_coast_dialogue_data.h"
/* Species indices are stable across saves: see data/species.json. */
const uint16_t fish_base_prices[FISH_SPECIES_COUNT]={20,20,50,20,50,50,50,20,72,60,50,200,500,200,600,48,50,60,40,20,160,20,68,180,52,20,50,180,50,20,20,40,120,65,20,1600,55,105,20,20,50,50,20,20,200,20,20,1,80,80};
const fish_container_t fish_containers[FISH_CONTAINER_COUNT]={
    {"小木桶",10000,0},{"大木桶",20000,300},{"储鱼箱",35000,750},{"大型储鱼箱",80000,1800}};
typedef struct {uint8_t slot,index;uint16_t price;} shop_item_t;
/* sell, rods, lines, floats, five packs of bait, return */
static const shop_item_t items[FISH_SHOP_COUNT]={
    {5,0,0},{0,1,550},{0,2,1800},{1,0,120},{1,2,450},{2,1,90},{2,2,120},
    {3,0,8},{3,1,6},{3,2,10},{3,3,18},{3,4,20},{4,1,0},{4,2,0},{5,1,0},{5,2,0},{3,6,30},{3,7,24},{3,8,600},{6,0,35},{1,1,200},{1,3,600},
    {0,FISH_LONG_ROD,3500},{FISH_SHOP_UTILITY,0,160},{FISH_SHOP_DRINK,1,28},{FISH_SHOP_FOOD,0,25},{FISH_SHOP_SERVICE,3,FISH_CABIN_PRICE},
    {FISH_SHOP_LINE,FISH_MARINE_LINE,1200},{FISH_SHOP_UTILITY,1,FISH_LAMP_PRICE},{FISH_SHOP_ROD,FISH_HEAVY_SEA_ROD,6000},{FISH_SHOP_TOOL,0,120},{FISH_SHOP_TOOL,1,180},{FISH_SHOP_CONTAINER,3,0}};
/* Stable item IDs are saved/tested independently of each vendor's order. */
static const uint8_t wetland_tackle[]={0,1,2,3,20,4,5,6,7,8,9,10,11,12,13,14,15};
static const uint8_t city_tackle[]={0,1,2,22,3,20,4,21,6,10,11,16,17,18,13,FISH_SHOP_LARGE_BOX,14,15};
static const uint8_t coast_tackle[]={0,FISH_SHOP_HEAVY_ROD,FISH_SHOP_PE_LINE,FISH_SHOP_MARINE_LINE,FISH_SHOP_LAMP,FISH_SHOP_SHOVEL,FISH_SHOP_CROWBAR,FISH_SHOP_CABIN,FISH_SHOP_CARGO,FISH_SHOP_BACK};
static const uint8_t convenience[]={19,24,23,25,15};
unsigned fish_shop_count(const struct fish_game *g)
{return !g->map_index?FISH_WETLAND_SHOP_COUNT:g->map_index==2?sizeof(coast_tackle)+(fish_shop_free_line(g)?1:0):g->shop_venue==FISH_VENDOR_CONVENIENCE?sizeof(convenience):sizeof(city_tackle);}
unsigned fish_shop_item_at(const struct fish_game *g,unsigned position)
{
    if(position>=fish_shop_count(g))return FISH_SHOP_BACK;
    if(g->map_index==2 && fish_shop_free_line(g) && position>=sizeof(coast_tackle)-1)return position==sizeof(coast_tackle)-1?3:FISH_SHOP_BACK;
    return !g->map_index?wetland_tackle[position]:g->map_index==2?coast_tackle[position]:g->shop_venue==FISH_VENDOR_CONVENIENCE?convenience[position]:city_tackle[position];
}
unsigned fish_shop_position(const struct fish_game *g,unsigned item)
{for(unsigned i=0;i<fish_shop_count(g);i++)if(fish_shop_item_at(g,i)==item)return i;return fish_shop_count(g);}
void fish_economy_init(fish_economy_t *e)
{
    memset(e,0,sizeof(*e));e->rods=1;e->lines=2;e->floats=1;e->hanging_species=FISH_SPECIES_COUNT;
}
unsigned fish_hold_page_count(const fish_economy_t *e)
{
    unsigned count=e->hanging_species<FISH_SPECIES_COUNT;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) count+=e->count[i]>0;
    return count;
}
unsigned fish_hold_page_index(const fish_economy_t *e,unsigned page)
{
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) if(e->count[i]) {
        if(!page) return i;
        --page;
    }
    if(!page && e->hanging_species<FISH_SPECIES_COUNT) return FISH_SPECIES_COUNT;
    return FISH_SPECIES_COUNT+1;
}
uint32_t fish_catch_price(unsigned species,uint16_t size_mm)
{return fish_catch_price_on(0,species,size_mm);}
uint32_t fish_catch_price_on(unsigned map,unsigned species,uint16_t size_mm)
{
    if(species>=FISH_SPECIES_COUNT) return 0;
    if(fish_species_is_variant(species)) return fish_catch_price_on(map,fish_species_parent(species),size_mm)*10;
    const fish_species_t *s=fish_species_profile(map,species);
    unsigned typical=s->mean_x2_mm/2;
    /* Length squared gives carp a broad range without cubic weight explosions. */
    uint64_t n=(uint64_t)fish_base_prices[species]*size_mm*size_mm;
    uint32_t price=(uint32_t)((n+typical*typical/2)/(typical*typical));
    return price?price:1;
}
uint32_t fish_hold_weight(const fish_economy_t *e)
{
    uint32_t n=0;for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) n+=e->grams[i];return n;
}
uint32_t fish_hold_capacity(const fish_economy_t *e)
{
    return fish_containers[e->container<FISH_CONTAINER_COUNT?e->container:0].capacity_g;
}
uint32_t fish_hold_value(const fish_economy_t *e)
{
    uint32_t n=e->hanging_value;for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) n+=e->value[i];return n;
}
uint32_t fish_cabin_weight(const fish_economy_t *e)
{uint32_t n=0;for(unsigned i=0;i<FISH_COAST_COUNT;i++)n+=e->cabin_grams[i];return n;}
uint32_t fish_cabin_value(const fish_economy_t *e)
{uint32_t n=0;for(unsigned i=0;i<FISH_COAST_COUNT;i++)n+=e->cabin_value[i];return n;}
unsigned fish_cargo_count(const struct fish_game *g,unsigned i)
{return i<FISH_SPECIES_COUNT?g->economy.count[i]+(g->map_index==2 && i>=FISH_COAST_FIRST?g->economy.cabin_count[i-FISH_COAST_FIRST]:0):0;}
uint32_t fish_cargo_grams(const struct fish_game *g,unsigned i)
{return g->economy.grams[i]+(g->map_index==2 && i>=FISH_COAST_FIRST?g->economy.cabin_grams[i-FISH_COAST_FIRST]:0);}
uint32_t fish_cargo_value(const struct fish_game *g,unsigned i)
{return g->economy.value[i]+(g->map_index==2 && i>=FISH_COAST_FIRST?g->economy.cabin_value[i-FISH_COAST_FIRST]:0);}
uint32_t fish_cargo_total_value(const struct fish_game *g)
{return fish_hold_value(&g->economy)+(g->map_index==2?fish_cabin_value(&g->economy):0);}
unsigned fish_cargo_pages(const struct fish_game *g)
{unsigned n=g->economy.hanging_species<FISH_SPECIES_COUNT;for(unsigned i=0;i<FISH_SPECIES_COUNT;i++)n+=fish_cargo_count(g,i)>0;return n;}
unsigned fish_cargo_page_index(const struct fish_game *g,unsigned page)
{for(unsigned i=0;i<FISH_SPECIES_COUNT;i++)if(fish_cargo_count(g,i)){if(!page)return i;--page;}return !page && g->economy.hanging_species<FISH_SPECIES_COUNT?FISH_SPECIES_COUNT:FISH_SPECIES_COUNT+1;}
fish_cargo_result_t fish_hold_add(fish_economy_t *e,unsigned species,uint16_t size_mm,uint32_t grams)
{return fish_hold_add_on(e,0,species,size_mm,grams);}
static fish_cargo_result_t hold_add_priced(fish_economy_t *e,unsigned map,unsigned species,uint16_t size_mm,uint32_t grams,uint32_t price)
{
    if(species>=FISH_SPECIES_COUNT || !grams) return FISH_RELEASED;
    if(map==2 && e->cabin_owned && species>=FISH_COAST_FIRST && grams<=FISH_CABIN_CAPACITY_G && fish_cabin_weight(e)<=FISH_CABIN_CAPACITY_G-grams) {
        unsigned k=species-FISH_COAST_FIRST;if(e->cabin_count[k]==UINT16_MAX)return FISH_RELEASED;
        ++e->cabin_count[k];e->cabin_grams[k]+=grams;e->cabin_value[k]+=price;
        for(unsigned i=FISH_PORTABLE_PRICE_GROUPS;i<FISH_PRICE_GROUPS;i++)if(!e->prices[i].count || e->prices[i].price==price) {
            if(price<=UINT16_MAX && e->prices[i].count<UINT16_MAX)e->prices[i]=(fish_price_group_t){(uint16_t)price,(uint16_t)(e->prices[i].count+1)};
            break;
        }
        return FISH_CABIN_STORED;
    }
    uint32_t capacity=fish_hold_capacity(e),used=fish_hold_weight(e);
    if(used>capacity)return FISH_RELEASED;
    if(grams>capacity-used) {
        /* An upgraded container's remaining room decides whether to use the
         * empty stern slot for a >10kg fish, not its empty-container capacity. */
        if(grams<=FISH_HOLD_CAPACITY_G || e->hanging_species<FISH_SPECIES_COUNT) return FISH_RELEASED;
        e->hanging_species=(uint8_t)species;e->hanging_map=(uint8_t)map;e->hanging_size_mm=size_mm;
        e->hanging_grams=grams;e->hanging_value=price;return FISH_HANGING;
    }
    if(e->count[species]==UINT16_MAX) return FISH_RELEASED;
    ++e->count[species];e->grams[species]+=grams;e->value[species]+=price;
    for(unsigned i=0;i<FISH_PORTABLE_PRICE_GROUPS;i++) if(!e->prices[i].count || e->prices[i].price==price) {
        if(price<=UINT16_MAX && e->prices[i].count<UINT16_MAX) {
            e->prices[i].price=(uint16_t)price;++e->prices[i].count;
        }
        break;
    }
    return FISH_STORED;
}
bool fish_equipment_owned(const fish_economy_t *e,unsigned slot,unsigned index)
{
    if(slot==FISH_BAG_ROD) return index<FISH_ROD_COUNT && (e->rods&(1u<<index));
    if(slot==FISH_BAG_LINE) return index<FISH_LINE_COUNT && (e->lines&(1u<<index));
    if(slot==FISH_BAG_FLOAT) return index<FISH_FLOAT_COUNT && (e->floats&(1u<<index));
    if(slot==FISH_BAG_BAIT) return index==FISH_PLASTIC_BAIT || (index<FISH_BAIT_COUNT && e->bait_stock[index]);
    return slot==FISH_BAG_DEPTH && (index==0 || index==1 || index==FISH_DEPTH_OPTION_COUNT-1);
}
fish_cargo_result_t fish_hold_add_on(fish_economy_t *e,unsigned map,unsigned species,uint16_t size_mm,uint32_t grams)
{return hold_add_priced(e,map,species,size_mm,grams,fish_catch_price_on(map,species,size_mm));}

const char *fish_shop_name(unsigned item)
{
    if(item>=FISH_SHOP_COUNT) return "返回小径";
    if(item==FISH_SHOP_CABIN)return "鱼舱";
    if(item==FISH_SHOP_LAMP)return "诱鱼灯";
    const shop_item_t *s=&items[item];
    if(s->slot==0) return fish_rods[s->index].name;
    if(s->slot==1) return fish_line_sets[s->index].name;
    if(s->slot==2) return fish_floats[s->index].name;
    if(s->slot==3) return fish_baits[s->index].name;
    if(s->slot==4) return fish_containers[s->index].name;
    if(s->slot==FISH_SHOP_DRINK) return s->index?"泡面":"热拿铁";
    if(s->slot==FISH_SHOP_UTILITY) return "雨伞";
    if(s->slot==FISH_SHOP_FOOD)return "鱼罐头";
    if(s->slot==FISH_SHOP_TOOL)return s->index?"撬棍":"铲子";
    return s->index==0?"出售全部鱼获":s->index==1?"查看容器":"返回小径";
}
uint32_t fish_shop_price(unsigned item)
{
    if(item>=FISH_SHOP_COUNT) return 0;
    return items[item].slot==4?fish_containers[items[item].index].price:items[item].price;
}
unsigned fish_shop_kind(unsigned item) {return item<FISH_SHOP_COUNT?items[item].slot:FISH_SHOP_SERVICE;}
unsigned fish_shop_item_index(unsigned item) {return item<FISH_SHOP_COUNT?items[item].index:2;}
static void money_popup(struct fish_game *g,int32_t amount,bool aggregate)
{
    unsigned slot=0;uint32_t oldest=0;
    for(unsigned i=0;i<FISH_MONEY_POPUPS;i++) {
        fish_money_popup_t *p=&g->money.popup[i];
        if(!p->amount) {slot=i;break;}
        uint32_t age=g->now-p->at;if(age>=oldest) {oldest=age;slot=i;}
    }
    g->money.popup[slot]=(fish_money_popup_t){amount,g->now,aggregate};
}
void fish_money_update(struct fish_game *g)
{
    fish_money_feedback_t *m=&g->money;
    /* Leaving the shop cancels cosmetic feedback, never an economic action. */
    if(g->mode!=FISH_SHOP) {memset(m,0,sizeof(*m));return;}
    for(unsigned i=0;i<FISH_MONEY_POPUPS;i++)
        if(m->popup[i].amount && g->now-m->popup[i].at>=FISH_MONEY_LIFETIME_MS) m->popup[i].amount=0;
    if(g->now-m->at<FISH_MONEY_INTERVAL_MS) return;
    uint32_t amount=0;bool aggregate=false;
    if(m->legacy_value) {amount=m->legacy_value;m->legacy_value=0;aggregate=true;}
    else {
        while(m->next<FISH_PRICE_GROUPS && !m->sale[m->next].count) ++m->next;
        if(m->next<FISH_PRICE_GROUPS) {
            fish_price_group_t *s=&m->sale[m->next];amount=s->price;--s->count;
        } else {amount=m->hanging_value;m->hanging_value=0;}
    }
    if(amount) {money_popup(g,(int32_t)amount,aggregate);m->at=g->now;}
}
bool fish_shop_free_line(const struct fish_game *g)
{return g->economy.coins<fish_shop_price(3) && !(g->economy.lines&1);}
fish_merchant_reply_t fish_shop_trade(struct fish_game *g,unsigned item)
{
    if(!fish_game_shop_open(g)) return FISH_MERCHANT_CLOSED;
    if(item>=FISH_SHOP_COUNT || fish_shop_position(g,item)>=fish_shop_count(g)) return FISH_MERCHANT_EMPTY;
    fish_economy_t *e=&g->economy;const shop_item_t *s=&items[item];
    if(item==0) {
        uint32_t value=fish_cargo_total_value(g);
        if(!value) return FISH_MERCHANT_EMPTY;
        if(value>FISH_COIN_LIMIT-e->coins) return FISH_MERCHANT_FULL;
        fish_money_feedback_t *m=&g->money;
        memcpy(m->sale,e->prices,sizeof(m->sale));m->next=0;
        if(g->map_index!=2)memset(m->sale+FISH_PORTABLE_PRICE_GROUPS,0,(FISH_PRICE_GROUPS-FISH_PORTABLE_PRICE_GROUPS)*sizeof(m->sale[0]));
        uint32_t detailed=0;for(unsigned i=0;i<FISH_PRICE_GROUPS;i++) detailed+=(uint32_t)m->sale[i].price*m->sale[i].count;
        m->legacy_value=value-e->hanging_value-detailed;m->hanging_value=e->hanging_value;
        m->at=g->now-FISH_MONEY_INTERVAL_MS;
        /* First popup is immediate; following fish are paced by the frame clock. */
        fish_money_update(g);
        e->coins+=value;memset(e->count,0,sizeof(e->count));memset(e->grams,0,sizeof(e->grams));
        memset(e->value,0,sizeof(e->value));memset(e->prices,0,FISH_PORTABLE_PRICE_GROUPS*sizeof(e->prices[0]));e->hanging_species=FISH_SPECIES_COUNT;
        if(g->map_index==2){memset(e->cabin_count,0,sizeof(e->cabin_count));memset(e->cabin_grams,0,sizeof(e->cabin_grams));memset(e->cabin_value,0,sizeof(e->cabin_value));memset(e->prices+FISH_PORTABLE_PRICE_GROUPS,0,(FISH_PRICE_GROUPS-FISH_PORTABLE_PRICE_GROUPS)*sizeof(e->prices[0]));}
        e->hanging_map=0;e->hanging_size_mm=0;e->hanging_grams=0;e->hanging_value=0;
        g->save_requested=true;return FISH_MERCHANT_SOLD;
    }
    if(item==FISH_SHOP_CABIN) {
        if(e->cabin_owned)return FISH_MERCHANT_OWNED;
        if(e->coins<FISH_CABIN_PRICE)return FISH_MERCHANT_POOR;
        e->coins-=FISH_CABIN_PRICE;e->cabin_owned=true;money_popup(g,-FISH_CABIN_PRICE,false);g->save_requested=true;return FISH_MERCHANT_BOUGHT;
    }
    if(s->slot==FISH_SHOP_TOOL){
        if(g->forage.tool[s->index])return FISH_MERCHANT_OWNED;
        if(e->coins<s->price)return FISH_MERCHANT_POOR;
        e->coins-=s->price;g->forage.tool[s->index]=20;money_popup(g,-(int32_t)s->price,false);g->save_requested=true;return FISH_MERCHANT_BOUGHT;
    }
    if(item==FISH_SHOP_LAMP){
        if(g->comforts&FISH_COMFORT_LAMP)return FISH_MERCHANT_OWNED;
        if(e->coins<FISH_LAMP_PRICE)return FISH_MERCHANT_POOR;
        e->coins-=FISH_LAMP_PRICE;g->comforts|=FISH_COMFORT_LAMP;
        money_popup(g,-FISH_LAMP_PRICE,false);g->save_requested=true;return FISH_MERCHANT_BOUGHT;
    }
    if(s->slot==FISH_SHOP_SERVICE) return FISH_MERCHANT_EMPTY;
    if(s->slot<3 && fish_equipment_owned(e,s->slot,s->index)) return FISH_MERCHANT_OWNED;
    if(s->slot==FISH_SHOP_UTILITY && (g->comforts&FISH_COMFORT_UMBRELLA))return FISH_MERCHANT_OWNED;
    if(s->slot==FISH_SHOP_FOOD && g->cat.tins>=FISH_CAT_FOOD_LIMIT)return FISH_MERCHANT_FULL;
    if(s->slot==3 && !fish_bait_consumable(s->index) && e->bait_stock[s->index])return FISH_MERCHANT_OWNED;
    if(s->slot==3 && fish_bait_consumable(s->index) && e->bait_stock[s->index]>FISH_BAIT_STOCK_LIMIT-5) return FISH_MERCHANT_FULL;
    if(s->slot==4 && e->container>=s->index) return FISH_MERCHANT_OWNED;
    bool relief=item==3 && fish_shop_free_line(g);
    uint32_t price=relief?0:fish_shop_price(item);
    if(e->coins<price) return FISH_MERCHANT_POOR;
    e->coins-=price;
    money_popup(g,-(int32_t)price,false);
    if(s->slot==0) {e->rods|=(uint8_t)(1u<<s->index);g->rod_index=s->index;}
    else if(s->slot==1) {e->lines|=(uint8_t)(1u<<s->index);if(fish_tackle_compatible(g->rod_index,s->index))fish_game_equip_line(g,s->index);}
    else if(s->slot==2) {e->floats|=(uint8_t)(1u<<s->index);g->float_index=s->index;}
    else if(s->slot==3) {if(fish_bait_consumable(s->index))e->bait_stock[s->index]+=5;else e->bait_stock[s->index]=1;g->bait_index=s->index;}
    else if(s->slot==FISH_SHOP_UTILITY)g->comforts|=FISH_COMFORT_UMBRELLA;
    else if(s->slot==FISH_SHOP_FOOD)++g->cat.tins;
    else if(s->slot==FISH_SHOP_DRINK) {
        g->comforts=(uint8_t)((g->comforts&~FISH_COMFORT_NOODLES)|(s->index?FISH_COMFORT_NOODLES:0));
        g->latte_remaining_ms=s->index?FISH_NOODLES_DURATION_MS:FISH_LATTE_DURATION_MS;
    }
    else {
        e->container=s->index;
        /* Move an existing hanging fish only when the larger container fits it.
         * The normal add path leaves all state untouched when space is short. */
        if(e->hanging_species<FISH_SPECIES_COUNT && e->hanging_grams<=fish_hold_capacity(e) &&
           hold_add_priced(e,e->hanging_map,e->hanging_species,e->hanging_size_mm,e->hanging_grams,e->hanging_value)==FISH_STORED) {
            e->hanging_species=FISH_SPECIES_COUNT;e->hanging_map=0;e->hanging_size_mm=0;e->hanging_grams=0;e->hanging_value=0;
        }
    }
    g->save_requested=true;return relief?FISH_MERCHANT_RELIEF:s->slot==FISH_SHOP_DRINK?(s->index?FISH_MERCHANT_NOODLES:FISH_MERCHANT_LATTE):FISH_MERCHANT_BOUGHT;
}
static unsigned merchant_context(const struct fish_game *g)
{
    switch(g->merchant_reply) {
        case FISH_MERCHANT_SOLD:return 3;
        case FISH_MERCHANT_BOUGHT:return 4;
        case FISH_MERCHANT_POOR:return 5;
        case FISH_MERCHANT_EMPTY:return 6;
        case FISH_MERCHANT_OWNED:return 7;
        case FISH_MERCHANT_FULL:return 8;
        case FISH_MERCHANT_CLOSED:return 0;
        case FISH_MERCHANT_RELIEF:return 14;
        default: {
            unsigned special=FISH_SPECIES_COUNT;uint32_t best=0;
            for(unsigned i=FISH_BASE_SPECIES_COUNT;i<FISH_LEGACY_SPECIES_COUNT;i++) {
                uint32_t value=g->economy.value[i]+(g->economy.hanging_species==i?g->economy.hanging_value:0);
                if(value>best) {best=value;special=i;}
            }
            if(special<FISH_SPECIES_COUNT) return 9+special-FISH_BASE_SPECIES_COUNT;
            if(g->merchant_hint) return 13;
            return g->economy.hanging_species<FISH_SPECIES_COUNT?2:fish_hold_value(&g->economy)?1:0;
        }
    }
}
static unsigned coast_context(const struct fish_game *g)
{
    if(g->merchant_reply==FISH_MERCHANT_GREETING) {
        if(fish_cargo_count(g,FISH_OARFISH) || g->economy.hanging_species==FISH_OARFISH)return 15;
        if(fish_cargo_count(g,FISH_PEACOCK))return 16;
    }
    return merchant_context(g);
}
const char *fish_merchant_says(const struct fish_game *g)
{
    if(g->map_index==2)return fish_coast_dialogue[coast_context(g)].lines[g->merchant_line_variant&1];
    if(g->map_index && g->shop_venue==FISH_VENDOR_CONVENIENCE)
        return fish_city_dialogue[g->merchant_reply==FISH_MERCHANT_LATTE?14:g->merchant_reply==FISH_MERCHANT_NOODLES?15:merchant_context(g)].lines[g->merchant_line_variant&1];
    return (g->map_index?fish_city_monger_dialogue:fish_merchant_dialogue)[merchant_context(g)].lines[g->merchant_line_variant&1];
}
unsigned fish_merchant_pose(const struct fish_game *g)
{
    if(g->map_index==2)return fish_coast_dialogue[coast_context(g)].pose;
    if(g->map_index && g->shop_venue==FISH_VENDOR_CONVENIENCE)return g->merchant_reply==FISH_MERCHANT_LATTE?4:g->merchant_reply==FISH_MERCHANT_POOR?2:0;
    return (g->map_index?fish_city_monger_dialogue:fish_merchant_dialogue)[merchant_context(g)].pose;
}
