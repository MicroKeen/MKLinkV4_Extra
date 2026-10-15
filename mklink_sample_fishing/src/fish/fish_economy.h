#ifndef FISH_ECONOMY_H
#define FISH_ECONOMY_H
#include "fish_species.h"
#include "fish_equipment.h"
enum {FISH_HOLD_CAPACITY_G=10000,FISH_COIN_LIMIT=1000000000,FISH_BAIT_STOCK_LIMIT=999,
      FISH_SHOP_COUNT=16,FISH_SHOP_CONTAINER_FIRST=12,FISH_SHOP_CARGO=14,FISH_SHOP_BACK=15,
      FISH_CONTAINER_COUNT=3,FISH_PLASTIC_BAIT=5,FISH_SPECIAL_BAIT_COUNT=5};
enum {FISH_SHOP_OPEN_MINUTE=420,FISH_SHOP_CLOSE_MINUTE=1080};
enum {FISH_SHOP_ROD,FISH_SHOP_LINE,FISH_SHOP_FLOAT,FISH_SHOP_BAIT,FISH_SHOP_CONTAINER,FISH_SHOP_SERVICE};
typedef struct {const char *name;uint32_t capacity_g;uint16_t price;} fish_container_t;
typedef enum {FISH_STORED,FISH_HANGING,FISH_RELEASED} fish_cargo_result_t;
typedef enum {FISH_MERCHANT_GREETING,FISH_MERCHANT_SOLD,FISH_MERCHANT_BOUGHT,
              FISH_MERCHANT_POOR,FISH_MERCHANT_EMPTY,FISH_MERCHANT_OWNED,FISH_MERCHANT_FULL,FISH_MERCHANT_CLOSED} fish_merchant_reply_t;
typedef struct {
    uint32_t coins;
    uint8_t rods,lines,floats;
    uint8_t container;
    uint16_t bait_stock[FISH_SPECIAL_BAIT_COUNT];
    uint16_t count[FISH_SPECIES_COUNT];
    uint32_t grams[FISH_SPECIES_COUNT],value[FISH_SPECIES_COUNT];
    uint8_t hanging_species; /* FISH_SPECIES_COUNT means no hanging fish. */
    uint16_t hanging_size_mm;
    uint32_t hanging_grams,hanging_value;
} fish_economy_t;
struct fish_game;
extern const uint16_t fish_base_prices[FISH_SPECIES_COUNT];
extern const fish_container_t fish_containers[FISH_CONTAINER_COUNT];
uint32_t fish_hold_capacity(const fish_economy_t *e);
void fish_economy_init(fish_economy_t *e);
uint32_t fish_catch_price(unsigned species,uint16_t size_mm);
uint32_t fish_hold_weight(const fish_economy_t *e);
uint32_t fish_hold_value(const fish_economy_t *e);
fish_cargo_result_t fish_hold_add(fish_economy_t *e,unsigned species,uint16_t size_mm,uint32_t grams);
bool fish_equipment_owned(const fish_economy_t *e,unsigned slot,unsigned index);
const char *fish_shop_name(unsigned item);
uint32_t fish_shop_price(unsigned item);
unsigned fish_shop_kind(unsigned item);
unsigned fish_shop_item_index(unsigned item);
fish_merchant_reply_t fish_shop_trade(struct fish_game *g,unsigned item);
const char *fish_merchant_says(const struct fish_game *g);
unsigned fish_merchant_pose(const struct fish_game *g);
#endif
