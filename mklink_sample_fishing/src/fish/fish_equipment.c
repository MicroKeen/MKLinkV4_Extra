#include "fish_equipment.h"
#include "fish_rules.h"

unsigned fish_line_span_cm(unsigned horizontal,unsigned depth)
{
    uint32_t n=horizontal*horizontal+depth*depth,root=0,bit=1u<<30;
    while(bit>n)bit>>=2;
    while(bit){if(n>=root+bit){n-=root+bit;root=(root>>1)+bit;}else root>>=1;bit>>=2;}
    return root;
}
bool fish_line_bottom_allowed(unsigned line,unsigned bottom)
{return line<FISH_LINE_COUNT && (line>=2 || bottom<(line?1000u:500u));}
unsigned fish_line_resolve_layer(unsigned line,unsigned bottom,unsigned horizontal,unsigned requested)
{
    if(line>=FISH_LINE_COUNT)return 0;
    if(requested>2)requested=2;
    for(unsigned layer=requested;layer;--layer){
        unsigned depth=layer==2?bottom:bottom/2;
        if((layer!=2 || fish_line_bottom_allowed(line,bottom)) &&
           fish_line_span_cm(horizontal,depth)<=fish_line_sets[line].distance_limit_cm)return layer;
    }
    return 0;
}

uint8_t fish_line_bite_percent(unsigned index,uint32_t weight_g)
{
    if(index==FISH_PE_LINE)return weight_g<=1000?4:weight_g<=5000?18:weight_g<=15000?60:90;
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
