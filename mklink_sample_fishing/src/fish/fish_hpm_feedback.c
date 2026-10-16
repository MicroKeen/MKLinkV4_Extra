#include "fish_hpm_feedback.h"
#include "beep_control.h"

enum { HOOK_HZ=160, HOOK_10MS=6, BURST_HZ=140, BURST_10MS=3,
       BURST_INTERVAL_MS=240, DUTY_PERCENT=20,
       CASH_HZ=660, CASH_TAIL_HZ=1100, CASH_10MS=4, CASH_TAIL_10MS=8, CASH_TAIL_AT_MS=50 };
static bool fighting,bursting,shopping;
static uint8_t cash_note,sound_mode;
static uint32_t pulse_at,cash_at,coins;

void fish_hpm_feedback_init(void)
{
    fighting=false;bursting=false;shopping=false;cash_note=0;
    pulse_at=0;cash_at=0;coins=0;sound_mode=FISH_SOUND_ALL;
    Beep_PWM_Init();
    Beep_PWM_frequency(0,0,0);
}

void fish_hpm_feedback_update(const fish_game_t *game)
{
    bool shop=game->mode==FISH_SHOP;
    if(sound_mode!=game->sound_mode) {
        Beep_PWM_frequency(0,0,0);cash_note=0;fighting=false;bursting=false;
        sound_mode=game->sound_mode;
    }
    bool traded=sound_mode==FISH_SOUND_ALL && shop && shopping && game->economy.coins!=coins &&
        (game->merchant_reply==FISH_MERCHANT_BOUGHT || game->merchant_reply==FISH_MERCHANT_SOLD);
    shopping=shop;coins=game->economy.coins;
    if(sound_mode==FISH_SOUND_OFF) {fighting=false;bursting=false;return;}
    if(game->mode!=FISH_FIGHT) {
        if(fighting) Beep_PWM_frequency(0,0,0);
        fighting=false;bursting=false;
        if(traded) {
            Beep_PWM_frequency(CASH_HZ,DUTY_PERCENT,CASH_10MS);
            cash_note=1;cash_at=game->now;
        } else if(shop && cash_note==1 && game->now-cash_at>=CASH_TAIL_AT_MS) {
            Beep_PWM_frequency(CASH_TAIL_HZ,DUTY_PERCENT,CASH_TAIL_10MS);
            cash_note=2;cash_at=game->now;
        } else if(shop && cash_note==2 && game->now-cash_at>=CASH_TAIL_10MS*10) {
            cash_note=0;
        } else if(!shop && cash_note) {
            Beep_PWM_frequency(0,0,0);cash_note=0;
        }
        return;
    }
    if(cash_note) {Beep_PWM_frequency(0,0,0);cash_note=0;}
    bool burst=fish_game_burst_active(game);
    if(!fighting) {
        Beep_PWM_frequency(HOOK_HZ,DUTY_PERCENT,HOOK_10MS);
        pulse_at=game->now;
    } else if(burst && (!bursting || game->now-pulse_at>=BURST_INTERVAL_MS)) {
        Beep_PWM_frequency(BURST_HZ,DUTY_PERCENT,BURST_10MS);
        pulse_at=game->now;
    } else if(!burst && bursting) {
        Beep_PWM_frequency(0,0,0);
    }
    fighting=true;bursting=burst;
}
