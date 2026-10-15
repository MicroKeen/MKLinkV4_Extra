#include "fish_game.h"
#include <assert.h>
#include <stdio.h>

static void hook(fish_game_t *g,unsigned id,uint16_t size,unsigned rod,unsigned line)
{
    fish_game_init(g,0);g->mode=FISH_BITE;g->phase_at=0;
    g->rod_index=(uint8_t)rod;fish_game_equip_line(g,line);
    g->pending_species=(uint8_t)id;g->pending_size_mm=size;
    g->phase_at-=fish_species[id].strike_delay_ms;
    g->pending_weight_g=fish_species_weight_g(&fish_species[id],size);
    g->cast_start_x=162;g->cast_distance=fish_rods[rod].max_cast_px;
    g->cast_x=g->cast_start_x+g->cast_distance;
    fish_game_input(g,FISH_PRESS,0,0);fish_game_input(g,FISH_RELEASE,0,0);
}

typedef struct {int tension;bool burst,warning;} observation_t;

static unsigned play(fish_game_t *g,unsigned delay)
{
    observation_t history[5];int actions[5]={0};unsigned pays=0;
    for(unsigned i=0;i<5;i++) history[i]=(observation_t){g->tension,false,false};
    for(unsigned tick=1;tick<=1200 && g->mode==FISH_FIGHT;tick++) {
        uint32_t t=tick*100;fish_game_update(g,t);if(g->mode!=FISH_FIGHT) break;
        history[tick%5]=(observation_t){g->tension,fish_game_burst_active(g),fish_game_burst_warning(g)};
        observation_t seen=history[(tick+5-delay)%5];
        int estimate=seen.tension+(int)delay*(seen.burst?4:2);
        /* Only visible cues and the player's own recent actions; no hidden force. */
        for(unsigned back=1;back<=delay;back++) {
            int own=actions[(tick+5-back)%5];estimate+=own>0?4:own<0?-10:0;
        }
        int steps=estimate>=g->line_strength*70/100?-1:
            (seen.burst||seen.warning) && estimate>=g->line_strength/2?0:1;
        actions[tick%5]=steps;if(steps<0) ++pays;
        fish_game_input(g,FISH_ROTATE,steps,t);
    }
    return pays;
}

int main(void)
{
    setvbuf(stdout,NULL,_IONBF,0);
    unsigned cases=0,landed=0,failed=0,payout_cases=0,longest=0;
    /* One detent per 100ms, with genuinely delayed tension/cue observations.
     * No species/weight knowledge, predictive force access or teleport reeling. */
    for (unsigned id=0;id<FISH_SPECIES_COUNT;id++) {
        const fish_species_t *s=&fish_species[id];
        uint16_t sizes[11]={s->min_mm,(s->mean_x2_mm+1)/2,s->max_mm};
        for (unsigned i=0;i<4;i++) {sizes[3+i*2]=s->star_min_mm[i]-1;sizes[4+i*2]=s->star_min_mm[i];}
        unsigned species_longest=0;
        for (unsigned size=0;size<11;size++) for (unsigned rod=0;rod<FISH_ROD_COUNT;rod++)
        for (unsigned line=0;line<FISH_LINE_COUNT;line++) for (unsigned delay=0;delay<=4;delay+=2) {
            fish_game_t g;hook(&g,id,sizes[size],rod,line);
            unsigned pays=play(&g,delay);
            if(g.mode==FISH_CAUGHT) {
                assert(g.catches==1 && g.last_species==id && g.loss==FISH_LOSS_NONE);++landed;
                if(g.phase_at>longest) longest=g.phase_at;
                if(g.phase_at>species_longest) species_longest=g.phase_at;
            } else {
                assert(g.mode==FISH_LOST && g.catches==0 &&
                    (g.loss==FISH_LOSS_BREAK || g.loss==FISH_LOSS_RANGE || g.loss==FISH_LOSS_SLACK));
                ++failed;
            }
            /* Ordinary sizes and suitable thick tackle must remain playable. */
            if(line==2 || sizes[size]==s->min_mm) assert(g.mode==FISH_CAUGHT);
            ++cases;payout_cases+=pays>0;
            if (id==1 && sizes[size]==s->star_min_mm[2] && rod==FISH_DEFAULT_ROD && delay==4)
                printf("four_star_trial line=%u reaction_ms=400 landing_ms=%u payout_detents=%u loss=%u\n",line,g.mode==FISH_CAUGHT?g.phase_at:0,pays,g.loss);
        }
        printf("%s longest_landing_ms=%u\n",s->id,species_longest);
    }
    /* Repeat the same 4-star carp matchup with different hook dispositions.
     * Outcomes are audit counts, never claimed to be human success rates. */
    for(unsigned line=0;line<FISH_LINE_COUNT;line++) {
        unsigned wins=0,losses=0;
        for(unsigned seed=1;seed<=128;seed++) {
            fish_game_t g;hook(&g,1,fish_species[1].star_min_mm[2],1,line);
            /* Re-hook with an independent identity seed, keeping equipment fixed. */
            g.mode=FISH_BITE;g.phase_at=0-fish_species[1].strike_delay_ms;g.random_state=seed*0x9e3779b9u;
            fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,0);
            assert(g.struggle_percent>=85 && g.struggle_percent<=120);
            play(&g,4);if(g.mode==FISH_CAUGHT) ++wins;else {assert(g.loss!=FISH_LOSS_NONE);++losses;}
        }
        printf("four_star_seeds line=%u seeds=128 landed=%u lost=%u\n",line,wins,losses);
        if(line==0) assert(wins>0 && losses>0);
        if(line==2) assert(wins==128);
    }
    /* Small fish: no large fixed burst bonus. Calm animals never make splash bursts. */
    for (unsigned id=0;id<FISH_SPECIES_COUNT;id++) {
        const fish_species_t *s=&fish_species[id];fish_game_t g;
        hook(&g,id,s->min_mm,FISH_DEFAULT_ROD,FISH_DEFAULT_LINE_SET);
        assert(g.base_pull<=30);
        for (unsigned t=0;t<=5000;t+=100) {
            fish_game_update(&g,t);assert(g.mode==FISH_FIGHT && g.tension<=60);
            if (s->fight_style!=FISH_FIGHT_ACTIVE) assert(fish_game_splash(&g)==0);
        }
        if (s->fight_style!=FISH_FIGHT_ACTIVE) {
            hook(&g,id,s->max_mm,FISH_DEFAULT_ROD,FISH_DEFAULT_LINE_SET);
            for (unsigned t=0;t<=30000;t+=100) {
                fish_game_update(&g,t);assert(g.mode==FISH_FIGHT && g.tension<85);
                assert(fish_game_splash(&g)==0);
                if (s->fight_style==FISH_FIGHT_PASSIVE)
                    assert(!fish_game_burst_active(&g) && !fish_game_burst_warning(&g) && g.reel_units==0);
            }
        }
    }
    /* A large fish loads the line without any retrieval; warning permits rescue. */
    fish_game_t idle,rescued;hook(&idle,3,fish_species[3].max_mm,1,0);rescued=idle;
    bool warned=false;unsigned warning_at=0;
    for (unsigned t=100;t<=20000 && idle.mode==FISH_FIGHT;t+=100) {
        fish_game_update(&idle,t);
        if (!warned && idle.tension>=idle.line_strength*85/100) {warned=true;warning_at=t;}
    }
    assert(warned && idle.loss==FISH_LOSS_BREAK && idle.now-warning_at>=600);
    for (unsigned t=100;t<=warning_at+400;t+=100) fish_game_update(&rescued,t);
    assert(rescued.mode==FISH_FIGHT);
    int before=rescued.tension;
    for (unsigned t=warning_at+500;t<=warning_at+1100;t+=100) {
        fish_game_input(&rescued,FISH_ROTATE,t==warning_at+500?-3:-1,t);assert(rescued.mode==FISH_FIGHT);
    }
    assert(rescued.tension<before);
    for (unsigned t=warning_at+1200;t<=warning_at+2000;t+=100)
        fish_game_input(&rescued,FISH_ROTATE,rescued.tension>=rescued.line_strength*70/100?-1:0,t);
    assert(rescued.mode==FISH_FIGHT && rescued.overload_ms==0);
    /* Concealment affects actual waiting time, rather than normalized species
     * weights that could never increase the total rate of bites. */
    const unsigned weights[]={50,500,5000};
    for (unsigned group=0;group<3;group++) {
        uint64_t sums[FISH_LINE_COUNT]={0};
        for (unsigned line=0;line<FISH_LINE_COUNT;line++) {
            uint32_t random=0x62b713u;
            for (unsigned n=0;n<100000;n++) {
                unsigned wait=fish_line_bite_wait(&random,line,weights[group]);
                assert(wait>=fish_rules.bite_attempt_ms && wait<=fish_rules.bite_attempt_ms*fish_rules.bite_attempt_limit);
                sums[line]+=wait;
            }
        }
        assert(sums[0]<sums[1] && sums[1]<sums[2]);
        printf("bite_weight_g=%u mean_wait_ms thin=%llu balanced=%llu thick=%llu\n",weights[group],
            (unsigned long long)(sums[0]/100000),(unsigned long long)(sums[1]/100000),(unsigned long long)(sums[2]/100000));
    }
    uint16_t four_star=fish_species[1].star_min_mm[2];
    assert(fish_rules.bite_attempt_ms==2860 && fish_rules.bite_attempt_limit==22);
    uint64_t normal_wait=0,glow_wait=0;
    for(unsigned n=1;n<=10000;n++) {
        uint32_t normal=n*0x9e3779b9u,glow=normal;
        unsigned a=fish_line_bite_wait(&normal,1,500),b=fish_line_bite_wait_bonus(&glow,1,500,10);
        assert(b<=a);normal_wait+=a;glow_wait+=b;
    }
    assert(glow_wait<normal_wait && glow_wait*100>normal_wait*80);
    printf("night_glow_mean_wait_ms normal=%llu glow=%llu\n",
        (unsigned long long)(normal_wait/10000),(unsigned long long)(glow_wait/10000));
    uint32_t big_carp_weight=fish_species_weight_g(&fish_species[1],four_star);
    assert(fish_line_response_percent(1,big_carp_weight)>=170);
    assert(fish_line_response_percent(2,big_carp_weight)<100);
    hook(&idle,1,four_star,1,1);
    for (unsigned t=100;t<=10000 && idle.mode==FISH_FIGHT;t+=100)
        fish_game_input(&idle,FISH_ROTATE,1,t);
    assert(idle.loss==FISH_LOSS_BREAK); /* Continuous retrieval cannot brute-force this matchup. */
    hook(&idle,1,fish_species[1].max_mm,1,0);
    idle.reel_units=10000-idle.line_capacity_units;idle.tension=idle.line_strength;
    int locked_tension=idle.tension;
    fish_game_input(&idle,FISH_ROTATE,-40,0);
    assert(idle.tension==locked_tension && fish_game_distance(&idle)==idle.line_capacity_units);
    fish_game_update(&idle,800);assert(idle.loss==FISH_LOSS_BREAK);
    printf("four_star_carp_mm=%u weight_g=%lu response thin=%u balanced=%u thick=%u; balanced continuous retrieve breaks\n",
        four_star,(unsigned long)big_carp_weight,fish_line_response_percent(0,big_carp_weight),
        fish_line_response_percent(1,big_carp_weight),fish_line_response_percent(2,big_carp_weight));
    assert(failed>0 && landed+failed==cases);
    printf("PASS: %u matchups, %u landed, %u lost, %u used payout, max landing %ums; small/calm limits, finite line and warning rescue\n",
        cases,landed,failed,payout_cases,longest);
}
