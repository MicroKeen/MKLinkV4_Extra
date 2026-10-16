#include "fish_game.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    fish_game_t g;
    fish_game_init(&g, 0);
    fish_game_input(&g,FISH_ROTATE,1,0); assert(g.boat_x==150 && g.boat_speed==1);
    fish_game_update(&g,1000); assert(g.boat_x==160); /* 10 px/s after just one detent. */
    fish_game_input(&g,FISH_ROTATE,1,1000); assert(g.boat_speed==2);
    fish_game_update(&g,2000); assert(g.boat_x==180); /* 20 px/s fast gear. */
    fish_game_input(&g,FISH_ROTATE,-1,2000); assert(g.boat_speed==0);
    fish_game_update(&g,3000); assert(g.boat_x==180);
    fish_game_input(&g,FISH_ROTATE,-1,3000); assert(g.boat_speed==-1 && g.facing==-1);
    fish_game_update(&g,4000); assert(g.boat_x==170);
    fish_game_input(&g,FISH_PRESS,0,4000); assert(g.boat_speed==0);
    fish_game_input(&g,FISH_RELEASE,0,4100); assert(g.mode==FISH_BAG);
    fish_game_update(&g,9000); assert(g.boat_x==170); /* No motion accumulated in the bag. */
    fish_game_init(&g,0);
    fish_game_input(&g, FISH_PRESS, 0, 30);
    fish_game_input(&g, FISH_RELEASE, 0, 100); assert(g.mode == FISH_BAG);
    fish_game_input(&g, FISH_ROTATE, FISH_BAG_VISIBLE_COUNT-1, 110);
    fish_game_input(&g, FISH_PRESS, 0, 120);
    fish_game_input(&g, FISH_RELEASE, 0, 150); assert(g.mode==FISH_MOVE);
    fish_game_input(&g, FISH_ROTATE, 1, 160); assert(g.boat_speed==1 && g.facing == 1);
    fish_game_input(&g, FISH_PRESS, 0, 200);
    fish_game_update(&g, 480); assert(g.mode == FISH_CHARGE && fish_game_power(&g) == 0);
    fish_game_input(&g, FISH_ROTATE, 20, 600); assert(g.boat_x == 150 && !g.boat_speed);
    fish_game_update(&g, 1600); assert(fish_game_power(&g) == 100);
    fish_game_input(&g, FISH_RELEASE, 0, 1700);
    assert(g.mode == FISH_CAST && g.cast_distance == 88 && g.cast_x == 250);
    fish_game_update(&g, 2350); assert(g.mode == FISH_WAIT);
    uint32_t bite_at=2350+g.bite_wait_ms;
    fish_game_update(&g,bite_at); assert(g.mode == FISH_BITE);
    fish_game_input(&g, FISH_ROTATE, 5, bite_at+10); assert(g.mode==FISH_BITE && g.reel==0);
    bite_at+=fish_species[g.pending_species].strike_delay_ms;
    fish_game_input(&g, FISH_PRESS, 0, bite_at+20); assert(g.mode==FISH_FIGHT);
    fish_game_input(&g, FISH_RELEASE, 0, bite_at+30); assert(g.mode==FISH_FIGHT);
    for (uint32_t t=bite_at+50;t<bite_at+45000 && g.mode==FISH_FIGHT;t+=100) {
        fish_game_update(&g,t);
        fish_game_input(&g,FISH_ROTATE,g.tension>=75?-2:1,t);
    }
    assert(g.mode == FISH_CAUGHT && g.catches == 1);
    assert(g.last_size_mm>=fish_species[g.last_species].min_mm);
    assert(g.last_size_mm<=fish_species[g.last_species].max_mm);
    assert(g.species_catches[g.last_species]==1);
    assert(g.last_weight_g==g.pending_weight_g && g.last_weight_g>0);
    uint32_t next=g.phase_at+4000;
    fish_game_update(&g,next); assert(g.mode == FISH_AIM);
    /* Down/up between frames must still produce a charged cast. */
    fish_game_input(&g, FISH_PRESS, 0, next+30);
    fish_game_input(&g, FISH_RELEASE, 0, next+530); assert(g.mode == FISH_CAST && g.cast_distance < 116);
    fish_game_update(&g,next+1180);
    fish_game_input(&g, FISH_PRESS, 0, next+1230);
    fish_game_input(&g, FISH_RELEASE, 0, next+1280); assert(g.mode == FISH_AIM);
    fish_game_input(&g, FISH_PRESS, 0, next+1330);
    fish_game_input(&g, FISH_RELEASE, 0, next+1380); assert(g.mode == FISH_BAG);
    fish_game_input(&g, FISH_ROTATE, FISH_BAG_VISIBLE_COUNT-1, next+1390);
    fish_game_input(&g, FISH_PRESS, 0, next+1400);
    fish_game_input(&g, FISH_RELEASE, 0, next+1410); assert(g.mode==FISH_AIM);
    fish_game_input(&g,FISH_ROTATE,-2,next+1430);
    fish_game_update(&g,next+2430);
    assert(g.boat_x == FISH_BOAT_LEFT && fish_game_camera(&g) == FISH_BOAT_LEFT-120);
    assert(!g.boat_speed);
    fish_game_input(&g, FISH_PRESS, 0, next+2530);
    fish_game_input(&g, FISH_RELEASE, 0, next+4030); assert(g.cast_x == FISH_WATER_LEFT+4);
    /* Cruise cadence is independent of rendering and safe across timer wrap. */
    fish_game_t a,b;
    fish_game_init(&a,0);fish_game_input(&a,FISH_ROTATE,1,0);b=a;
    for(unsigned t=10;t<=1000;t+=10) fish_game_update(&a,t);
    fish_game_update(&b,1000);assert(a.boat_x==b.boat_x && a.boat_motion_q8==b.boat_motion_q8);
    fish_game_input(&b,FISH_ROTATE,1,1000);
    fish_game_update(&b,60000);assert(b.boat_x==180); /* At most 1 second of background catch-up. */
    b.boat_x=FISH_BOAT_RIGHT-1;fish_game_update(&b,60100);assert(b.boat_x==FISH_BOAT_RIGHT && !b.boat_speed);
    fish_game_init(&b,UINT32_MAX-49);fish_game_input(&b,FISH_ROTATE,-1,UINT32_MAX-49);
    fish_game_update(&b,50);assert(b.boat_x==149);
    /* Unsigned timestamps work across the millisecond counter wrap. */
    fish_game_init(&g, UINT32_MAX - 500);
    fish_game_input(&g, FISH_PRESS, 0, UINT32_MAX - 500);
    fish_game_input(&g, FISH_RELEASE, 0, 1000); assert(g.mode == FISH_CAST);
    fish_game_init(&g,0);g.mode=FISH_BAG;g.bag_index=FISH_BAG_GUIDE;
    fish_game_input(&g,FISH_ROTATE,1,0);assert(g.bag_index==FISH_BAG_SOUND);
    fish_game_input(&g,FISH_ROTATE,2,0);assert(g.bag_index==FISH_BAG_BACK);
    fish_game_input(&g,FISH_ROTATE,-1,0);assert(g.bag_index==FISH_BAG_SAVE);
    fish_game_input(&g,FISH_ROTATE,-2,0);assert(g.bag_index==FISH_BAG_GUIDE);
    fish_game_input(&g,FISH_ROTATE,4,0);assert(g.bag_index==FISH_BAG_ROD);
    g.bag_index=FISH_BAG_SOUND;g.save_requested=false;
    fish_game_input(&g,FISH_PRESS,0,10);fish_game_input(&g,FISH_RELEASE,0,11);assert(g.bag_editing);
    fish_game_input(&g,FISH_ROTATE,1,12);assert(g.sound_mode==FISH_SOUND_OFF);
    fish_game_input(&g,FISH_ROTATE,1,13);assert(g.sound_mode==FISH_SOUND_FISH);
    fish_game_input(&g,FISH_PRESS,0,14);fish_game_input(&g,FISH_RELEASE,0,15);
    assert(!g.bag_editing && g.save_requested && g.mode==FISH_BAG);
    puts("PASS: gameplay, bag order and sound editing/save request");
    return 0;
}
