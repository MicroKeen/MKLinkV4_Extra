#ifndef FISH_FORAGE_H
#define FISH_FORAGE_H
#include <stdint.h>
#include <stdbool.h>
struct fish_game;
enum {FISH_FORAGE_NODES=12,FISH_FORAGE_DURABILITY=20,FISH_FORAGE_REACH=12,FISH_FORAGE_NOTICE=48,
      FISH_FORAGE_QTE_MS=6500,FISH_FORAGE_REFRESH_MINUTE=240,
      FISH_FORAGE_BAR_WIDTH=164,FISH_FORAGE_TARGET_LEFT=56,FISH_FORAGE_TARGET_WIDTH=52};
enum {FISH_FORAGE_QUIET,FISH_FORAGE_NEED_SHOVEL,FISH_FORAGE_NEED_BAR,FISH_FORAGE_ESCAPED,FISH_FORAGE_WORM_FULL};
typedef struct {
    uint32_t at;
    uint16_t taken,seen,size_mm;
    uint8_t day,ready,tool[2],node,reply,return_shore,amount,stored;
} fish_forage_t;
unsigned fish_forage_day(const struct fish_game *g);
unsigned fish_forage_count(const struct fish_game *g,unsigned area);
int fish_forage_x(const struct fish_game *g,unsigned node);
unsigned fish_forage_species(const struct fish_game *g,unsigned node);
bool fish_forage_exists(const struct fish_game *g,unsigned node);
bool fish_forage_visible(const struct fish_game *g,unsigned node);
int fish_forage_near(const struct fish_game *g);
void fish_forage_update(struct fish_game *g);
bool fish_forage_start(struct fish_game *g);
bool fish_forage_input(struct fish_game *g,int event,int steps);
unsigned fish_forage_slider(const struct fish_game *g);
unsigned fish_forage_slider_width(const struct fish_game *g);
bool fish_forage_hit(const struct fish_game *g);
const char *fish_forage_message(const struct fish_game *g);
#endif
