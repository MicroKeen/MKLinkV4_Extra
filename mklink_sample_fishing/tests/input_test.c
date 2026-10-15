#include "fish_hpm_input.h"
#include "key_task.h"
#include <assert.h>
#include <stdio.h>

ref_gui_t g_tMyGUI;
static fish_game_t game;
static uint32_t now;
static bool button;
static int presses,releases,last_steps;
static void driver_init(void){}
static bool driver_read(key_id_t id){assert(id==KEY_ID_0);return button;}
key_driver_t key_driver={driver_init,driver_read};
int64_t get_system_ticks(void){return now;}
void fish_scene_input(fish_event_t event,int steps,uint32_t at)
{
    if(event==FISH_PRESS)presses++;
    if(event==FISH_RELEASE)releases++;
    if(event==FISH_ROTATE)last_steps=steps;
    fish_game_input(&game,event,steps,at);
}
bool rg_send_key_evt(ref_gui_t *gui,uint8_t event,uint16_t key)
{
    ngy_msg_key_evt_t message={{event},key};
    return fish_hpm_handle_message(gui,&message.core);
}
static void wheel(uint16_t value)
{
    ngy_msg_gesture_evt_t m={.core={NGY_MSG_GESTURE_EVT_WHEEL},
        .tRegion={.tLocation={0,(int16_t)value}}};
    assert(fish_hpm_handle_message(&g_tMyGUI,&m.core));
}
static void run_ms(unsigned count)
{
    while(count--){now++;key_module_display(1);fish_game_update(&game,now);}
}
int main(void)
{
    fish_game_init(&game,0);
    wheel(1);assert(last_steps==1 && game.boat_speed==1 && game.boat_x==150);
    run_ms(1000);assert(game.boat_x==160);
    wheel(2);assert(last_steps==1 && game.boat_speed==2);
    run_ms(1000);assert(game.boat_x==180);
    wheel(1);assert(last_steps==-1 && game.boat_speed==0);
    run_ms(1000);assert(game.boat_x==180);
    wheel(0);assert(last_steps==-1 && game.boat_speed==-1);
    run_ms(1000);assert(game.boat_x==170);
    wheel(0);assert(last_steps==0 && game.boat_speed==-1);
    wheel(6);assert(last_steps==6 && game.boat_speed==2); /* Missed intermediate messages. */
    wheel(32767);wheel(32768);assert(last_steps==1); /* Signed 16-bit boundary. */
    wheel(65535);wheel(0);assert(last_steps==1);
    wheel(65535);assert(last_steps==-1);wheel(0);
    ngy_msg_core_t unknown={1};assert(!fish_hpm_handle_message(&g_tMyGUI,&unknown));
    ngy_msg_key_evt_t other={{NGY_MSG_KEY_EVT_DOWN},0x52};
    assert(!fish_hpm_handle_message(&g_tMyGUI,&other.core));

    fish_game_init(&game,now);key_init();
    button=true;run_ms(70);button=false;run_ms(150);
    assert(game.mode==FISH_BAG && presses==1 && releases==1);
    assert(rg_send_key_evt(&g_tMyGUI,NGY_MSG_KEY_EVT_PRESSED,RG_KEY_DOWN));
    assert(rg_send_key_evt(&g_tMyGUI,NGY_MSG_KEY_EVT_REPEAT,RG_KEY_DOWN));
    assert(rg_send_key_evt(&g_tMyGUI,NGY_MSG_KEY_EVT_DOUBLE_PRESSED,RG_KEY_DOWN));
    assert(game.mode==FISH_BAG && presses==1 && releases==1);
    wheel(FISH_BAG_VISIBLE_COUNT-1);button=true;run_ms(60);button=false;run_ms(150);
    assert(game.mode==FISH_MOVE && presses==2 && releases==2);

    button=true;run_ms(300);assert(game.mode==FISH_CHARGE);
    run_ms(1200);assert(fish_game_power(&game)==100);
    button=false;run_ms(20);assert(game.mode==FISH_CAST && releases==3);
    run_ms(650);assert(game.mode==FISH_WAIT);
    while(game.mode==FISH_WAIT)run_ms(1);
    assert(game.mode==FISH_BITE);
    run_ms(fish_species[game.pending_species].strike_delay_ms);
    button=true;run_ms(20);assert(game.mode==FISH_FIGHT);
    button=false;run_ms(20);assert(releases==4 && game.mode==FISH_FIGHT);
    uint16_t value=FISH_BAG_VISIBLE_COUNT-1;
    int before=game.reel_units;
    wheel(++value);assert(game.reel_units>before);
    game.tension=50;before=game.reel_units;
    wheel(--value);assert(game.reel_units<before && game.tension<50);

    puts("PASS: original key state machine -> GUI events -> game; wheel direction, dropped events, wraps, short press, long release, hook, reel, payout");
    return 0;
}
