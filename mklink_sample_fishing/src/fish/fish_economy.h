#ifndef FISH_ECONOMY_H
#define FISH_ECONOMY_H
#include "fish_species.h"
#include "fish_equipment.h"
enum {FISH_HOLD_CAPACITY_G=10000,FISH_COIN_LIMIT=1000000000,FISH_BAIT_STOCK_LIMIT=999,
      FISH_SHOP_COUNT=22,FISH_WETLAND_SHOP_COUNT=17,FISH_SHOP_CONTAINER_FIRST=12,FISH_SHOP_CARGO=14,FISH_SHOP_BACK=15,
      FISH_SHOP_SNAIL=16,FISH_SHOP_CORN=17,FISH_SHOP_LIFELIKE=18,FISH_SHOP_LATTE=19,FISH_SHOP_STANDARD_LINE=20,FISH_SHOP_PE_LINE=21,
      FISH_CONTAINER_COUNT=3,FISH_PLASTIC_BAIT=5,FISH_SPECIAL_BAIT_COUNT=5};
enum {FISH_PRICE_GROUPS=256,FISH_MONEY_POPUPS=4,FISH_MONEY_INTERVAL_MS=250,FISH_MONEY_LIFETIME_MS=900};
typedef struct {uint16_t price,count;} fish_price_group_t;
typedef struct {int32_t amount;uint32_t at;bool aggregate;} fish_money_popup_t;
typedef struct {
    fish_price_group_t sale[FISH_PRICE_GROUPS];
    uint16_t next;
    uint32_t legacy_value,hanging_value,at;
    fish_money_popup_t popup[FISH_MONEY_POPUPS];
} fish_money_feedback_t;
enum {FISH_SHOP_OPEN_MINUTE=420,FISH_SHOP_CLOSE_MINUTE=1080};
enum {FISH_SHOP_ROD,FISH_SHOP_LINE,FISH_SHOP_FLOAT,FISH_SHOP_BAIT,FISH_SHOP_CONTAINER,FISH_SHOP_SERVICE,FISH_SHOP_DRINK};
enum {FISH_VENDOR_TACKLE,FISH_VENDOR_CONVENIENCE,FISH_LATTE_DURATION_MS=360000,FISH_CITY_STALL_X=-92};
static inline bool fish_bait_consumable(unsigned bait) {return bait<FISH_BAIT_COUNT && bait!=FISH_PLASTIC_BAIT;}
typedef struct {const char *name;uint32_t capacity_g;uint16_t price;} fish_container_t;
typedef enum {FISH_STORED,FISH_HANGING,FISH_RELEASED} fish_cargo_result_t;
typedef enum {FISH_MERCHANT_GREETING,FISH_MERCHANT_SOLD,FISH_MERCHANT_BOUGHT,
              FISH_MERCHANT_POOR,FISH_MERCHANT_EMPTY,FISH_MERCHANT_OWNED,FISH_MERCHANT_FULL,FISH_MERCHANT_CLOSED,FISH_MERCHANT_LATTE,FISH_MERCHANT_RELIEF} fish_merchant_reply_t;
typedef struct {
    uint32_t coins;
    uint8_t rods,lines,floats;
    uint8_t container;
    uint16_t bait_stock[FISH_BAIT_COUNT]; /* Stable bait IDs; slot5 is the unlimited plastic lure. */
    uint16_t count[FISH_SPECIES_COUNT];
    uint32_t grams[FISH_SPECIES_COUNT],value[FISH_SPECIES_COUNT];
    uint8_t hanging_species; /* FISH_SPECIES_COUNT means no hanging fish. */
    uint8_t hanging_map;
    uint16_t hanging_size_mm;
    uint32_t hanging_grams,hanging_value;
    /* Equal prices share a count. Existing aggregate-only cargo stays intact. */
    fish_price_group_t prices[FISH_PRICE_GROUPS];
} fish_economy_t;
struct fish_game;
extern const uint16_t fish_base_prices[FISH_SPECIES_COUNT];
extern const fish_container_t fish_containers[FISH_CONTAINER_COUNT];
uint32_t fish_hold_capacity(const fish_economy_t *e);
void fish_economy_init(fish_economy_t *e);
uint32_t fish_catch_price(unsigned species,uint16_t size_mm);
uint32_t fish_catch_price_on(unsigned map,unsigned species,uint16_t size_mm);
uint32_t fish_hold_weight(const fish_economy_t *e);
uint32_t fish_hold_value(const fish_economy_t *e);
unsigned fish_hold_page_count(const fish_economy_t *e);
unsigned fish_hold_page_index(const fish_economy_t *e,unsigned page);
fish_cargo_result_t fish_hold_add(fish_economy_t *e,unsigned species,uint16_t size_mm,uint32_t grams);
fish_cargo_result_t fish_hold_add_on(fish_economy_t *e,unsigned map,unsigned species,uint16_t size_mm,uint32_t grams);
bool fish_equipment_owned(const fish_economy_t *e,unsigned slot,unsigned index);
const char *fish_shop_name(unsigned item);
uint32_t fish_shop_price(unsigned item);
unsigned fish_shop_kind(unsigned item);
unsigned fish_shop_item_index(unsigned item);
unsigned fish_shop_count(const struct fish_game *g);
unsigned fish_shop_item_at(const struct fish_game *g,unsigned position);
unsigned fish_shop_position(const struct fish_game *g,unsigned item);
fish_merchant_reply_t fish_shop_trade(struct fish_game *g,unsigned item);
bool fish_shop_free_line(const struct fish_game *g);
void fish_money_update(struct fish_game *g);
const char *fish_merchant_says(const struct fish_game *g);
unsigned fish_merchant_pose(const struct fish_game *g);
#endif
