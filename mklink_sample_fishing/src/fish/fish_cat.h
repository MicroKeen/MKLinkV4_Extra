#ifndef FISH_CAT_H
#define FISH_CAT_H
#include <stdint.h>
#include <stdbool.h>
enum {FISH_CAT_X=-208,FISH_CAT_FRIEND_MAX=50,FISH_CAT_DAILY_FEEDS=2,FISH_CAT_VISIT_ACTIONS=3,
      FISH_CAT_FOOD_LIMIT=999,FISH_CAT_SMALL_FISH_G=500,FISH_CAT_WALK_LIMIT=28,FISH_CAT_PERSONAL_SPACE=28,
      FISH_CAT_HOURLY_CHANCE_PERCENT=65};
enum {FISH_CAT_READY=1,FISH_CAT_VISIBLE=2,FISH_CAT_STROLL_MASK=12,
      FISH_CAT_SIT=0,FISH_CAT_WALK_RIGHT=4,FISH_CAT_WALK_LEFT=8,FISH_CAT_SLEEP=12,FISH_CAT_BLOCKED=16};
enum {FISH_CAT_QUIET,FISH_CAT_PETTED,FISH_CAT_FED,FISH_CAT_NO_FOOD,FISH_CAT_FULL,
      FISH_CAT_TIRED,FISH_CAT_JOINED,FISH_CAT_FISH_FED};
typedef struct {
    uint16_t tins,encounter;
    uint8_t friendship,feeds,day,actions;
    uint8_t menu,reply;
    uint8_t flags;
    int8_t walk_x;
    uint32_t reply_at,stroll_at;
} fish_cat_t;
struct fish_game;
bool fish_cat_owned(const struct fish_game *g);
bool fish_cat_present(const struct fish_game *g);
bool fish_cat_near(const struct fish_game *g);
unsigned fish_cat_day(const struct fish_game *g);
unsigned fish_cat_feeds_left(const struct fish_game *g);
void fish_cat_update(struct fish_game *g);
void fish_cat_interact(struct fish_game *g,unsigned action);
bool fish_cat_catch_menu(const struct fish_game *g);
bool fish_cat_feed_catch(struct fish_game *g);
bool fish_cat_input(struct fish_game *g,int event,int steps);
unsigned fish_cat_pose(const struct fish_game *g,bool aboard);
int fish_cat_world_x(const struct fish_game *g);
bool fish_cat_facing_left(const struct fish_game *g);
const char *fish_cat_message(const struct fish_game *g);
#endif
