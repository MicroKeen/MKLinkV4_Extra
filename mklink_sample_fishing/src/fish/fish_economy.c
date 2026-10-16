#include "fish_economy.h"
#include "fish_game.h"
#include <string.h>
#include "fish_merchant_dialogue_data.h"
/* Species indices are stable across saves: see data/species.json. */
const uint16_t fish_base_prices[FISH_SPECIES_COUNT]={8,12,16,12,16,28,30,6,72,60,10,120,160,120,600};
const fish_container_t fish_containers[FISH_CONTAINER_COUNT]={
    {"小木桶",10000,0},{"大木桶",20000,300},{"储鱼箱",35000,750}};
typedef struct {uint8_t slot,index;uint16_t price;} shop_item_t;
/* sell, rods, lines, floats, five packs of bait, return */
static const shop_item_t items[FISH_SHOP_COUNT]={
    {5,0,0},{0,1,550},{0,2,1800},{1,0,35},{1,2,100},{2,1,90},{2,2,120},
    {3,0,8},{3,1,6},{3,2,10},{3,3,18},{3,4,20},{4,1,0},{4,2,0},{5,1,0},{5,2,0}};
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
{
    if(species>=FISH_SPECIES_COUNT) return 0;
    if(species>=FISH_BASE_SPECIES_COUNT) return fish_catch_price(fish_species_parent(species),size_mm)*10;
    const fish_species_t *s=&fish_species[species];
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
fish_cargo_result_t fish_hold_add(fish_economy_t *e,unsigned species,uint16_t size_mm,uint32_t grams)
{
    if(species>=FISH_SPECIES_COUNT || !grams) return FISH_RELEASED;
    uint32_t price=fish_catch_price(species,size_mm);
    uint32_t capacity=fish_hold_capacity(e);
    if(grams>capacity) {
        if(e->hanging_species<FISH_SPECIES_COUNT) return FISH_RELEASED;
        e->hanging_species=(uint8_t)species;e->hanging_size_mm=size_mm;
        e->hanging_grams=grams;e->hanging_value=price;return FISH_HANGING;
    }
    if(grams>capacity-fish_hold_weight(e) || e->count[species]==UINT16_MAX) return FISH_RELEASED;
    ++e->count[species];e->grams[species]+=grams;e->value[species]+=price;
    for(unsigned i=0;i<FISH_PRICE_GROUPS;i++) if(!e->prices[i].count || e->prices[i].price==price) {
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
    if(slot==FISH_BAG_BAIT) return index==FISH_PLASTIC_BAIT || (index<FISH_SPECIAL_BAIT_COUNT && e->bait_stock[index]);
    return slot==FISH_BAG_DEPTH && index<FISH_DEPTH_OPTION_COUNT;
}
const char *fish_shop_name(unsigned item)
{
    if(item>=FISH_SHOP_COUNT) return "返回小径";
    const shop_item_t *s=&items[item];
    if(s->slot==0) return fish_rods[s->index].name;
    if(s->slot==1) return fish_line_sets[s->index].name;
    if(s->slot==2) return fish_floats[s->index].name;
    if(s->slot==3) return fish_baits[s->index].name;
    if(s->slot==4) return fish_containers[s->index].name;
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
fish_merchant_reply_t fish_shop_trade(struct fish_game *g,unsigned item)
{
    if(!fish_game_shop_open(g)) return FISH_MERCHANT_CLOSED;
    if(item>=FISH_SHOP_COUNT) return FISH_MERCHANT_EMPTY;
    fish_economy_t *e=&g->economy;const shop_item_t *s=&items[item];
    if(item==0) {
        uint32_t value=fish_hold_value(e);
        if(!value) return FISH_MERCHANT_EMPTY;
        if(value>FISH_COIN_LIMIT-e->coins) return FISH_MERCHANT_FULL;
        fish_money_feedback_t *m=&g->money;
        memcpy(m->sale,e->prices,sizeof(m->sale));m->next=0;
        uint32_t detailed=0;for(unsigned i=0;i<FISH_PRICE_GROUPS;i++) detailed+=(uint32_t)e->prices[i].price*e->prices[i].count;
        m->legacy_value=value-e->hanging_value-detailed;m->hanging_value=e->hanging_value;
        m->at=g->now-FISH_MONEY_INTERVAL_MS;
        /* First popup is immediate; following fish are paced by the frame clock. */
        fish_money_update(g);
        e->coins+=value;memset(e->count,0,sizeof(e->count));memset(e->grams,0,sizeof(e->grams));
        memset(e->value,0,sizeof(e->value));memset(e->prices,0,sizeof(e->prices));e->hanging_species=FISH_SPECIES_COUNT;
        e->hanging_size_mm=0;e->hanging_grams=0;e->hanging_value=0;
        g->save_requested=true;return FISH_MERCHANT_SOLD;
    }
    if(s->slot>4) return FISH_MERCHANT_EMPTY;
    if(s->slot<3 && fish_equipment_owned(e,s->slot,s->index)) return FISH_MERCHANT_OWNED;
    if(s->slot==3 && e->bait_stock[s->index]>FISH_BAIT_STOCK_LIMIT-5) return FISH_MERCHANT_FULL;
    if(s->slot==4 && e->container>=s->index) return FISH_MERCHANT_OWNED;
    uint32_t price=fish_shop_price(item);
    if(e->coins<price) return FISH_MERCHANT_POOR;
    e->coins-=price;
    money_popup(g,-(int32_t)price,false);
    if(s->slot==0) {e->rods|=(uint8_t)(1u<<s->index);g->rod_index=s->index;}
    else if(s->slot==1) {e->lines|=(uint8_t)(1u<<s->index);fish_game_equip_line(g,s->index);}
    else if(s->slot==2) {e->floats|=(uint8_t)(1u<<s->index);g->float_index=s->index;}
    else if(s->slot==3) {e->bait_stock[s->index]+=5;g->bait_index=s->index;}
    else {
        e->container=s->index;
        /* Move an existing hanging fish only when the larger container fits it.
         * The normal add path leaves all state untouched when space is short. */
        if(e->hanging_species<FISH_SPECIES_COUNT && e->hanging_grams<=fish_hold_capacity(e) &&
           fish_hold_add(e,e->hanging_species,e->hanging_size_mm,e->hanging_grams)==FISH_STORED) {
            e->hanging_species=FISH_SPECIES_COUNT;e->hanging_size_mm=0;e->hanging_grams=0;e->hanging_value=0;
        }
    }
    g->save_requested=true;return FISH_MERCHANT_BOUGHT;
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
        default: {
            unsigned special=FISH_SPECIES_COUNT;uint32_t best=0;
            for(unsigned i=FISH_BASE_SPECIES_COUNT;i<FISH_SPECIES_COUNT;i++) {
                uint32_t value=g->economy.value[i]+(g->economy.hanging_species==i?g->economy.hanging_value:0);
                if(value>best) {best=value;special=i;}
            }
            if(special<FISH_SPECIES_COUNT) return 9+special-FISH_BASE_SPECIES_COUNT;
            if(g->merchant_hint) return 13;
            return g->economy.hanging_species<FISH_SPECIES_COUNT?2:fish_hold_value(&g->economy)?1:0;
        }
    }
}
const char *fish_merchant_says(const struct fish_game *g)
{
    return fish_merchant_dialogue[merchant_context(g)].lines[g->merchant_line_variant&1];
}
unsigned fish_merchant_pose(const struct fish_game *g)
{
    return fish_merchant_dialogue[merchant_context(g)].pose;
}
