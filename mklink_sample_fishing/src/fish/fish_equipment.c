#include "fish_equipment.h"
#include "fish_rules.h"

uint8_t fish_line_bite_percent(unsigned index,uint32_t weight_g)
{
    return fish_line_sets[index].bite_percent[weight_g<=100?0:weight_g<=1000?1:2];
}

unsigned fish_line_response_percent(unsigned index,uint32_t weight_g)
{
    const fish_line_set_t *line=&fish_line_sets[index];
    unsigned response=line->response_percent;
    if (weight_g>line->comfortable_weight_g)
        response+=(weight_g-line->comfortable_weight_g)*fish_rules.overweight_response_gain/line->comfortable_weight_g;
    return response>fish_rules.max_line_response_percent?fish_rules.max_line_response_percent:response;
}

uint16_t fish_line_bite_wait(uint32_t *state,unsigned index,uint32_t weight_g)
{
    return fish_line_bite_wait_bonus(state,index,weight_g,0);
}
uint16_t fish_line_bite_wait_bonus(uint32_t *state,unsigned index,uint32_t weight_g,unsigned bonus_percent)
{
    unsigned chance=fish_line_bite_percent(index,weight_g),attempts=0;
    chance=chance*(100+bonus_percent)/100;
    if(chance>100) chance=100;
    do {
        uint32_t r=*state?*state:0x6d2b79f5u;
        r^=r<<13;r^=r>>17;r^=r<<5;*state=r;
        ++attempts;
        if (r%100<chance) break;
    } while (attempts<fish_rules.bite_attempt_limit);
    return (uint16_t)(attempts*fish_rules.bite_attempt_ms);
}
