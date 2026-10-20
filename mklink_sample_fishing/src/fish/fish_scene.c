#include "fish_scene.h"
#include "fish_save.h"
#include "fish_save_storage.h"
#include "fish_resources.h"
#include "fish_media.h"
#include "assets/game_icon.h"
#include "fish_asset_layout.h"
#include "fish_route_view.h"
#include "fish_hpm_input.h"
#include "fish_hpm_feedback.h"
#include "assets/fish_assets.h"
#include "assets/species/species_assets.h"
#include "assets/remarks_font.h"
#include "assets/equipment/equipment_art.h"
#include "assets/equipment/backpack_art.h"
#include "assets/shop/shop_art.h"
#include "assets/shop/interior_indexed.h"
#include "assets/shop/list/list_goods.h"
#include "assets/city_vendor_indexed.h"
#include "assets/player/shore_player_art.h"
#include "fish_scene_layout.h"
#include "fish_shop_layout.h"
#include "fish_tackle_descriptions.h"
#include "fish_guide_data.h"
#include "fish_hold_portrait_bounds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define fish_smoke_test 0
#define fish_boot_smoke 0
/* Base-first derivation follows Helper/template/arm_2d_scene_template.h. */
typedef struct {
    implement(arm_2d_scene_t);
    fish_game_t game;
    fish_save_t save;
    fish_route_view_t route_view;
    int smoke_frame;
    bool resource_loading,resource_boot,media_started,media_complete;
    uint16_t portrait_scale_q8;
    uint16_t card_size_mm;
    uint32_t card_weight_g;
    arm_2d_size_t remarks_box, guide_name_box, hint_box, save_box;
    arm_2d_size_t bag_names[FISH_BAG_COUNT];
    arm_2d_op_trans_msk_t portrait_operation;
    arm_2d_op_trans_t backpack_operation;
    arm_2d_op_trans_t exterior_closed_operation;
    arm_2d_op_trans_t shop_detail_operation;
    arm_2d_op_trans_msk_t hanging_operation;
    arm_2d_op_trans_msk_t container_operation;
    arm_2d_op_trans_msk_t footpath_operation,exterior_operation,shore_player_operation,station_operation;
    arm_2d_op_trans_msk_t dock_operation[2];
    arm_2d_op_trans_msk_t hold_portrait_operation;
    arm_2d_op_trans_msk_t park_lamp_operation,park_light_operation;
    arm_2d_op_trans_msk_t cloud_operation;
} fish_scene_t;
_Static_assert(offsetof(fish_scene_t,use_as__arm_2d_scene_t)==0,"Scene base must be first");
static fish_scene_t scene;

static void weight_text(char *out,size_t capacity,uint32_t grams)
{
    if(grams>1000) {
        unsigned long tenths=grams/100+(grams%100>=50);
        snprintf(out,capacity,"%lu.%lu kg",tenths/10,tenths%10);
    } else snprintf(out,capacity,"%lu克",(unsigned long)grams);
}

#define RGB(r,g,b) ((((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3))
enum {
    INK=RGB(24,34,45), SKY=RGB(184,201,190), CLOUD=RGB(224,219,184),
    SUN=RGB(250,228,158), FAR=RGB(140,165,153), HILL=RGB(95,130,121),
    PINE=RGB(53,93,83), DARK_PINE=RGB(40,69,65), WATER=RGB(55,104,118),
    DEEP=RGB(43,80,100), RIPPLE=RGB(112,153,151), FOAM=RGB(178,194,171),
    WOOD=RGB(155,96,61), WOOD_LIGHT=RGB(206,153,95), CREAM=RGB(237,227,188),
    RED=RGB(195,105,75), GREEN=RGB(136,172,123), SHADOW=RGB(32,57,65),
    RPG_DARK=RGB(43,35,30), RPG_WOOD=RGB(91,64,46), RPG_EDGE=RGB(120,89,57),
    RPG_BRASS=RGB(173,133,76), RPG_PAPER=RGB(222,209,174), RPG_PAPER_SHADE=RGB(199,182,145),
    RPG_INK=RGB(60,47,37), RPG_MUTED=RGB(119,98,70), RPG_LIGHT=RGB(239,225,183)
};

/* Thin wrappers use Arm-2D's region clipping, including PFB intersections. */
static void box(const arm_2d_tile_t *tile, int x, int y, int w, int h, uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    arm_2d_region_t region = { .tLocation = {x,y}, .tSize = {w,h} };
    arm_2d_rgb16_fill_colour(tile, &region, color);
    ARM_2D_OP_WAIT_ASYNC();
}

static void line(const arm_2d_tile_t *tile, int x, int y, int x1, int y1, uint16_t color)
{
    int dx = abs(x1-x), sx = x < x1 ? 1 : -1;
    int dy = -abs(y1-y), sy = y < y1 ? 1 : -1, error = dx+dy;
    for (;;) {
        box(tile, x,y,1,1,color);
        if (x == x1 && y == y1) break;
        int twice = 2*error;
        if (twice >= dy) { error += dy; x += sx; }
        if (twice <= dx) { error += dx; y += sy; }
    }
}

static void text(const arm_2d_tile_t *tile, int x, int y, uint16_t color, const char *value)
{
    arm_2d_region_t region = { .tLocation = {x,y}, .tSize = {240-x,10} };
    arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)tile);
    arm_lcd_text_set_font(&ARM_2D_FONT_6x8.use_as__arm_2d_font_t);
    arm_lcd_text_set_draw_region(&region);
    arm_lcd_text_set_colour(color, INK);
    arm_lcd_text_location(0,0);
    arm_lcd_puts(value);
    ARM_2D_OP_WAIT_ASYNC();
}
static void ui_text(const arm_2d_tile_t *tile,int x,int y,int width,uint16_t color,uint16_t bg,const char *value)
{
    arm_2d_size_t size=arm_lcd_get_string_line_box(value,fish_remarks_font());
    if(fish_smoke_test && (size.iWidth>width || size.iHeight>19 || x<0 || x+size.iWidth>240)) {
        fprintf(stderr,"UI label does not fit: %s (%d/%d, height %d)\n",value,size.iWidth,width,size.iHeight);exit(1);
    }
    arm_2d_region_t region={.tLocation={x,y},.tSize={size.iWidth,19}};
    arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)tile);
    arm_lcd_text_set_font(fish_remarks_font());arm_lcd_text_set_draw_region(&region);
    arm_lcd_text_set_colour(color,bg);arm_lcd_text_location(0,0);arm_lcd_puts(value);
    ARM_2D_OP_WAIT_ASYNC();
}
static void special_frame(const arm_2d_tile_t *tile,int x,int y,int w,int h)
{
    static const uint16_t colors[]={RGB(255,113,98),RGB(255,208,80),RGB(110,210,135),RGB(79,205,226),RGB(139,154,246),RGB(225,123,221)};
    for(unsigned i=0;i<6;i++) {
        int a=w*(int)i/6,b=w*(int)(i+1)/6,c=h*(int)i/6,d=h*(int)(i+1)/6;
        box(tile,x+a,y,b-a,2,colors[i]);box(tile,x+a,y+h-2,b-a,2,colors[5-i]);
        box(tile,x,y+c,2,d-c,colors[5-i]);box(tile,x+w-2,y+c,2,d-c,colors[i]);
    }
}

static void masked_bitmap(const arm_2d_tile_t *target, const arm_2d_tile_t *source,
                          const arm_2d_tile_t *mask, int x, int y, bool flip)
{
    arm_2d_region_t region = { .tLocation={x,y}, .tSize=source->tRegion.tSize };
    arm_2d_rgb565_tile_copy_with_src_mask(source,mask,target,&region,
        flip ? ARM_2D_CP_MODE_X_MIRROR : ARM_2D_CP_MODE_COPY);
    ARM_2D_OP_WAIT_ASYNC();
}

static void scaled_species_bitmap(const arm_2d_tile_t *target, const arm_2d_tile_t *source,
                                 const arm_2d_tile_t *mask, int centre_x, int centre_y,
                                 bool flip, uint16_t scale_q8, arm_2d_op_trans_msk_t *operation)
{
    if (scale_q8==256) {
        masked_bitmap(target,source,mask,centre_x-source->tRegion.tSize.iWidth/2,
            centre_y-source->tRegion.tSize.iHeight/2,flip);
        return;
    }
    arm_2d_point_float_t target_centre={centre_x,centre_y};
    arm_fsm_rt_t rendered=arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)operation,target,NULL,&target_centre);
    if (fish_smoke_test && rendered<0) { fprintf(stderr,"Transform failed: %d\n",rendered); exit(1); }
    ARM_2D_OP_WAIT_ASYNC(operation);
}

extern const uint8_t fish_sky_horizon[240];

static int floor_divide(int value,int divisor)
{ return value>=0?value/divisor:-((-value+divisor-1)/divisor); }
static int horizon_at(int source_column)
{
    int width=480;
    source_column%=2*width;if(source_column<0) source_column+=2*width;
    if(source_column>=width) source_column=2*width-1-source_column;
    return FISH_ASSET_POINTER(FISH_ASSET_SKYLINE_OFFSET)[source_column/2];
}

/* The original lake skyline clips sky objects behind mountains and trees,
 * using the exact same shallow camera parallax and 2-pixel source grid. */
static void sky_rect(const arm_2d_tile_t *tile,int source_x,int x,int y,int w,int h,
                     uint16_t color,uint8_t opacity)
{
    int end=x+w;
    if(x<0) x=0;
    if(end>240) end=240;
    for(int col=x;col<end;) {
        int span=2-((source_x+col)&1);
        if(span>end-col) span=end-col;
        int bottom=horizon_at(source_x+col);
        if(bottom>y+h) bottom=y+h;
        if(bottom>y) {
            arm_2d_region_t region={.tLocation={col,y},.tSize={span,bottom-y}};
            if(arm_2d_helper_pfb_is_region_active(tile,&region,true)) {
                arm_2d_rgb565_fill_colour_with_opacity(tile,&region,
                    (arm_2d_color_rgb565_t){.tValue=color},opacity);
                ARM_2D_OP_WAIT_ASYNC();
            }
        }
        col+=span;
    }
}

static void sky_cloud(const arm_2d_tile_t *tile,int sky_source_x,int x)
{
    int start=x-2,end=x+182;
    if(start<0) start=0;
    if(end>240) end=240;
    arm_2d_point_float_t centre={x+90,54};
    for(int col=start;col<end;) {
        int bottom=horizon_at(sky_source_x+col),next=col+1;
        if(bottom>66) bottom=66;
        while(next<end) {
            int limit=horizon_at(sky_source_x+next);
            if(limit>66) limit=66;
            if(limit!=bottom) break;
            ++next;
        }
        if(bottom>42) {
            arm_2d_region_t sky={.tLocation={col,42},.tSize={next-col,bottom-42}};
            if(arm_2d_helper_pfb_is_region_active(tile,&sky,true)) {
                arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.cloud_operation,tile,&sky,&centre);
                ARM_2D_OP_WAIT_ASYNC(&scene.cloud_operation);
            }
        }
        col=next;
    }
}
static void night_moon(const arm_2d_tile_t *tile,const fish_game_t *game)
{
    unsigned minute=game->game_minute;
    if(minute>=360 && minute<1080) return;
    unsigned clear=255-fish_weather_rain(game);if(!clear)return;
    unsigned arc=(minute+360)%1440;if(arc>720) arc=720;
    int x=18+(int)arc*204/720,y=104-(int)(arc*(720-arc)*52/(360*360));
    int camera=fish_game_camera(game),source_x=floor_divide(camera,3);
    static const uint8_t crescent[8]={0x3c,0x0e,0x07,0x07,0x07,0x07,0x0e,0x3c};
    for(int row=0;row<8;row++) for(int col=0;col<8;col++) {
        if(crescent[row]&(1u<<col)) sky_rect(tile,source_x,x-8+col*2,y-8+row*2,2,2,
            col<=2?RGB(217,233,219):RGB(171,209,202),(uint8_t)clear);
    }
}

static void opacity_box(const arm_2d_tile_t *tile,int x,int y,int w,int h,uint16_t color,unsigned alpha);
/* Only two RGB565 scanlines are decoded. No full image/frame buffer. */
static uint16_t city_night_rows[480] __attribute__((aligned(4)));
static const arm_2d_tile_t city_night_row_tile={
    .tRegion={.tSize={240,2}},
    .tInfo={.bIsRoot=true,.bHasEnforcedColour=true,.tColourInfo={.chScheme=ARM_2D_COLOUR_RGB565}},
    .phwBuffer=city_night_rows
};
/* Palette decode shares the existing two scanlines with sky/interior drawing.
 * Colour-keyed Arm-2D copy clips native2x sprites to each active PFB block. */
static unsigned vendor_index(const fish_vendor_image_t *im,unsigned pixel)
{
    unsigned bit=pixel*im->bits,at=bit/8,value=im->indices[at];
    if(bit%8+im->bits>8)value|=(unsigned)im->indices[at+1]<<8;
    return (value>>(bit%8))&((1u<<im->bits)-1);
}
static void vendor_view(const arm_2d_tile_t *tile,const fish_vendor_image_t *im,int left,int top,unsigned width,unsigned height,unsigned first)
{
    for(unsigned y=first;y<height;y++) {
        arm_2d_region_t row={.tLocation={left,top+(int)y*2},.tSize={width*2,2}};
        if(!arm_2d_helper_pfb_is_region_active(tile,&row,true))continue;
        for(unsigned x=0;x<width;x++) {
            uint16_t color=im->palette[vendor_index(im,y*im->height/height*im->width+x*im->width/width)];
            city_night_rows[2*x]=city_night_rows[2*x+1]=color;
            city_night_rows[240+2*x]=city_night_rows[241+2*x]=color;
        }
        arm_2d_rgb16_tile_copy_with_colour_keying(&city_night_row_tile,tile,&row,0xf81f,ARM_2D_CP_MODE_COPY);
        ARM_2D_OP_WAIT_ASYNC();
    }
}
static void vendor_image(const arm_2d_tile_t *tile,const fish_vendor_image_t *im,int left,int top,unsigned first)
{vendor_view(tile,im,left,top,im->width,im->height,first);}
static void city_lights(const arm_2d_tile_t *tile,const fish_game_t *g,int source_x)
{
    unsigned night=fish_game_night_opacity(g);if(!g->map_index || !night)return;
    const uint8_t *data=FISH_ASSET_POINTER(FISH_ASSET_NIGHT_OFFSET);
    for(int y=0;y<240;y+=2) {
        arm_2d_region_t row={.tLocation={0,y},.tSize={240,2}};
        if(!arm_2d_helper_pfb_is_region_active(tile,&row,true))continue;
        for(int x=0;x<240;x++) {
            int column=source_x+x;
            if(column<0)column=-1-column;
            if(column>=480)column=959-column;
            unsigned index=data[128+(y/2)*240+column/2];if(index>=64)index=0;
            uint16_t color=(uint16_t)(data[index*2]|(unsigned)data[index*2+1]<<8);
            city_night_rows[x]=city_night_rows[240+x]=color;
        }
        arm_2d_rgb565_tile_copy_with_opacity(&city_night_row_tile,tile,&row,night*255/150);
        ARM_2D_OP_WAIT_ASYNC();
    }
    /* The authored palette keeps windows, signs and reflections aligned.
     * Only the sky effects below are procedural. */
    for(unsigned beam=0;beam<2;beam++) {
        int root_x=beam?375:184,base_y=beam?86:77;
        int swing=(g->now/70+beam*71)%240;swing=swing<120?swing-60:180-swing;
        for(int y=22;y<base_y;y+=2) {
            int height=base_y-y,width=2+height/6;
            int x=root_x-source_x+swing*height/60;
            sky_rect(tile,source_x,x-width/2,y,width,2,beam?RGB(164,116,255):RGB(88,154,255),night*20/150);
        }
    }
    for(unsigned i=0;i<8;i++)sky_rect(tile,source_x,(i*79+29)%480-source_x,34+(i*17)%35,1,1,RGB(136,157,202),night*130/150);
}

static void landscape(const arm_2d_tile_t *tile, const fish_game_t *game)
{
    unsigned rain=fish_weather_rain(game),clear=255-rain;
    int camera = fish_game_camera(game);
    int source_x=floor_divide(camera,3);
    const uint8_t *data=FISH_ASSET_POINTER(FISH_ASSET_DAY_OFFSET);
    for(int y=0;y<240;y+=2) {
        arm_2d_region_t row={.tLocation={0,y},.tSize={240,2}};
        if(!arm_2d_helper_pfb_is_region_active(tile,&row,true))continue;
        for(int x=0;x<240;x++) {
            int column=(source_x+x)%960;if(column<0)column+=960;
            if(column>=480)column=959-column;
            unsigned index=data[128+(y/2)*240+column/2];
            uint16_t color=(uint16_t)(data[index*2]|(unsigned)data[index*2+1]<<8);
            city_night_rows[x]=city_night_rows[240+x]=color;
        }
        arm_2d_rgb16_tile_copy(&city_night_row_tile,tile,&row,ARM_2D_CP_MODE_COPY);
        ARM_2D_OP_WAIT_ASYNC();
    }
    int sky_source_x=source_x;
    unsigned night=fish_game_night_opacity(game);
    if(night) {
        unsigned opacity=night*255/150;
        if(opacity>255) opacity=255;
        sky_rect(tile,sky_source_x,0,0,240,game->map_index?120:84,RGB(3,7,20),(uint8_t)opacity);
        /* Sparse fixed stars drift with the sky, never over the ridge. */
        for(unsigned i=0;i<24;i++) {
            int x=(int)((i*73+19)%480)-sky_source_x;
            int y=30+(int)((i*29+7)%46);
            sky_rect(tile,sky_source_x,x,y,i%5==0?2:1,1,
                i%3==0?RGB(173,184,192):RGB(112,135,160),(uint8_t)(opacity*clear/255));
            if(sky_source_x<0) sky_rect(tile,sky_source_x,-1-(int)((i*73+19)%480)-sky_source_x,y,i%5==0?2:1,1,
                i%3==0?RGB(173,184,192):RGB(112,135,160),(uint8_t)(opacity*clear/255));
        }
    }
    /* Independent transparent cloud layer: slow drift and shallow parallax. */
    unsigned minute=game->game_minute;
    bool sun=minute>=360 && minute<1080;
    unsigned arc=sun?minute-360:(minute+360)%1440;
    if(arc>720) arc=720;
    int celestial_x=18+(int)arc*204/720;
    int celestial_y=104-(int)(arc*(720-arc)*52/(360*360));
    /* Twelve strips suggest reflected light, with no framebuffer copy. */
    for(int i=0;i<12;i++) {
        int w=2+i/3,shift=(int)((game->now/180+i*13)%7)-3;
        if(!rain)box(tile,celestial_x-w+shift,132+i*5,w*2,1,sun?RGB(176,146,83):RGB(131,157,170));
        else if(clear)opacity_box(tile,celestial_x-w+shift,132+i*5,w*2,1,sun?RGB(176,146,83):RGB(131,157,170),clear);
    }
    int cloud_offset=(floor_divide(camera,4)+(int)(game->now/900))%300;
    if(cloud_offset<0) cloud_offset+=300;
    if(!(fish_smoke_test==27 && (scene.smoke_frame&1)))
        for (int x=-cloud_offset; x<240; x+=300) sky_cloud(tile,sky_source_x,x);
    if(sun) {
        for(int dy=-5;dy<=5;dy++) {
            int half=5-abs(dy)/2;
            sky_rect(tile,sky_source_x,celestial_x-half,celestial_y+dy,half*2,1,SUN,(uint8_t)clear);
        }
    }
    /* A few restrained animated pixel ripples keep the bitmap water alive. */
    for (int i=0;i<12;i++) {
        int x=(i*53+(int)(game->now/200)-camera/2)%250;
        if (x<0) x+=250;
        box(tile,x,150+(i*17)%48,4+i%4*2,1,RIPPLE);
    }
    city_lights(tile,game,source_x);
}

/* Fog ends at the far-water plane, before any bank, actor, tackle or HUD.
 * Two-pixel horizontal bands avoid full-image masks and transient allocations. */
static void weather_haze(const arm_2d_tile_t *tile,const fish_game_t *g)
{
    unsigned rain=fish_weather_rain(g),base=0,t=g->game_minute;
    if(g->map_index)base=t>=1200 || t<360?92:t<480?92*(480-t)/120:t>=1080?92*(t-1080)/120:0;
    unsigned mist=base+(255-base)*rain/255;if(!mist)return;
    int source_x=floor_divide(fish_game_camera(g),3);
    unsigned night=fish_game_night_opacity(g);
    uint16_t sky=RGB((93*(150-night)+21*night)/150,(117*(150-night)+33*night)/150,(128*(150-night)+47*night)/150);
    sky_rect(tile,source_x,0,42,240,100,sky,(uint8_t)(rain*176/255));
    unsigned r=(154*(150-night)+62*night)/150;
    unsigned green=(176*(150-night)+81*night)/150;
    unsigned b=(178*(150-night)+92*night)/150;
    uint16_t fog=RGB(r,green,b);
    int spread=g->map_index?8:0,radius=48+spread;
    unsigned drift=(g->now/900)%16;int centre=98+spread+(drift<8?(int)drift:15-(int)drift)/2;
    for(int y=44;y<144+spread*2;y+=2) {
        int distance=abs(y-centre);
        unsigned density=distance>=radius?0:(radius-distance)*78/radius;
        unsigned alpha=mist*density/255;if(!alpha)continue;
        arm_2d_region_t band={.tLocation={0,y},.tSize={240,2}};
        if(arm_2d_helper_pfb_is_region_active(tile,&band,true))opacity_box(tile,0,y,240,2,fog,alpha);
    }
    /* Clouded light also cools the far water without flattening the near plane. */
    opacity_box(tile,0,142+spread,240,61-spread,RGB(52,78,93),mist*18/255);
}
static void weather_rain(const arm_2d_tile_t *tile,const fish_game_t *g)
{
    unsigned rain=fish_weather_rain(g);if(!rain)return;
    uint32_t tick=g->now/40;unsigned count=(rain*36+254)/255;
    uint16_t color=fish_game_night_opacity(g)?RGB(143,170,181):RGB(184,210,214);
    for(unsigned i=0;i<count;i++) {
        int x=(int)((i*67+tick*(1+i%2)/3)%252)-6;
        int y=42+(int)((i*29+tick*(3+i%3))%160);
        arm_2d_region_t drop={.tLocation={x-2,y},.tSize={4,6}};
        if(!arm_2d_helper_pfb_is_region_active(tile,&drop,true))continue;
        unsigned alpha=rain*(i%3?86:132)/255;
        opacity_box(tile,x,y,1,2,color,alpha/2);
        opacity_box(tile,x-1,y+2,1,2,color,alpha);
        if(i%3==0)opacity_box(tile,x-2,y+4,1,2,color,alpha/2);
    }
    /* Sparse middle/near drops cross the mist sharply. Longer/faster/brighter
     * near streaks add depth; the original short rain remains at strength zero.
     * Fixed frame time keeps all PFB strips consistent without a particle pool. */
    unsigned strength=fish_weather_strength(g);
    for(unsigned plane=0;plane<2;plane++) {
        unsigned extra=(strength*(plane?8:12)+254)/255;
        unsigned alpha=rain*strength/255*(plane?224:160)/255;
        for(unsigned i=0;i<extra;i++) {
            int x=(int)((i*71+plane*41+tick*(plane?3:2))%268)-10;
            int y=42+(int)((i*37+plane*59+tick*((plane?11:6)+i%3))%156);
            int step=plane?5+(int)(i%3):3+(int)(i%2),width=plane?2:1;
            arm_2d_region_t drop={.tLocation={x-2,y},.tSize={width+2,step*3}};
            if(!arm_2d_helper_pfb_is_region_active(tile,&drop,true))continue;
            opacity_box(tile,x,y,width,step,color,alpha/3);
            opacity_box(tile,x-1,y+step,width,step,color,alpha);
            opacity_box(tile,x-2,y+step*2,width,step,color,alpha*2/3);
        }
    }
    int left=FISH_WATER_LEFT-fish_game_camera(g),right=FISH_WATER_RIGHT-fish_game_camera(g);
    for(unsigned i=0;i<16;i++) {
        unsigned age=(tick+i*19)%53;if(age>=12)continue;
        int x=(int)((i*83+31)%240),y=147+(int)((i*23)%51),w=2+age/2;
        if(x-w<left || x+w>=right)continue;
        unsigned alpha=rain*(12-age)*12/255;
        if(age<3)opacity_box(tile,x-1,y,2,1,color,alpha);
        else {
            int height=age>=7?2:1;
            opacity_box(tile,x-w+2,y-height,w*2-4,1,color,alpha/2);
            opacity_box(tile,x-w+2,y+height,w*2-4,1,color,alpha/2);
            opacity_box(tile,x-w,y-1,2,2,color,alpha);
            opacity_box(tile,x+w-2,y-1,2,2,color,alpha);
        }
    }
}

static void foreground(const arm_2d_tile_t *tile, const fish_game_t *game)
{
    int offset=(fish_game_camera(game)*5/4)%360;
    for (int x=-offset; x<240; x+=360)
        masked_bitmap(tile,&c_tileReedsRGB565,&c_tileReedsAlphaGRAY8,x,165+FISH_NEAR_Y,false);
}

static void city_dock_steps(const arm_2d_tile_t *tile,int camera,unsigned side)
{
    /* Four shallow treads on the dock's native2px grid. Their grey-green
     * timber shares the existing pier palette; bottom tread meets the water. */
    for(int step=0;step<4;step++) {
        int x=96+step*4,y=144+step*4;
        if(side)x=FISH_MAP_WIDTH-x-24;
        x-=camera;
        box(tile,x,y,24,4,RGB(72,80,64));
        box(tile,x,y,24,2,RGB(144,144,112));
        box(tile,x+4,y,2,2,RGB(112,120,88));
        box(tile,x+18,y,2,2,RGB(112,120,88));
    }
    int x=side?FISH_MAP_WIDTH-120-24:120;
    opacity_box(tile,x-camera,160,24,2,SHADOW,112);
    box(tile,x-camera-2,162,8,1,RIPPLE);box(tile,x-camera+18,162,8,1,RIPPLE);
}
static void banks_and_docks(const arm_2d_tile_t *tile,const fish_game_t *game)
{
    int camera=fish_game_camera(game);
    if(camera<56) {
        arm_2d_point_float_t centre;
        arm_2d_region_t land={.tLocation={-600-camera,0},.tSize={624,240}};
        for(int right=104;right>FISH_SHORE_CAMERA_LEFT;right-=96) {
            centre=(arm_2d_point_float_t){right-48-camera,156+FISH_NEAR_Y};
            arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.footpath_operation,tile,&land,&centre);
            ARM_2D_OP_WAIT_ASYNC(&scene.footpath_operation);
        }
        centre=(arm_2d_point_float_t){FISH_SHOP_X+10-camera,90+FISH_NEAR_Y};
        arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.exterior_operation,tile,NULL,&centre);
        ARM_2D_OP_WAIT_ASYNC(&scene.exterior_operation);
        if(game->map_index) {
            int x=FISH_CITY_STALL_X-camera;
            vendor_view(tile,&fish_image_CityStallExterior,x-50,72,50,38,0);
            if(fish_game_vendor_open(game,FISH_VENDOR_TACKLE)) {
                vendor_view(tile,&fish_image_MerchantGreeting,x-14,94,14,20,0);
                vendor_view(tile,&fish_image_CityStallExterior,x-50,72,50,38,23);
            }
        }
        if(!game->map_index && !fish_game_shop_open(game)) {
            centre=(arm_2d_point_float_t){FISH_SHOP_X+18-camera,109+FISH_NEAR_Y};
            arm_2dp_tile_transform_xy(&scene.exterior_closed_operation,tile,NULL,&centre);
            ARM_2D_OP_WAIT_ASYNC(&scene.exterior_closed_operation);
        }
    }
    if(camera>FISH_MAP_WIDTH-280) {
        arm_2d_region_t land={.tLocation={FISH_MAP_WIDTH-24-camera,0},
            .tSize={FISH_SHORE_CAMERA_RIGHT+240-(FISH_MAP_WIDTH-24),240}};
        for(int left=FISH_MAP_WIDTH-24;left<FISH_SHORE_CAMERA_RIGHT+240;left+=96) {
            arm_2d_point_float_t centre={left+48-camera,156+FISH_NEAR_Y};
            arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.footpath_operation,tile,&land,&centre);
            ARM_2D_OP_WAIT_ASYNC(&scene.footpath_operation);
        }
        /* Low foreground rails sit below the walking path/platform. All
         * geometry uses the same two-screen-pixel grid as the native tiles. */
        int rail_left=FISH_MAP_WIDTH+28,rail_right=FISH_SHORE_CAMERA_RIGHT+240;
        for(int x=rail_left;x<rail_right;x+=16) {
            box(tile,x-camera,150+FISH_NEAR_Y,6,16,RPG_DARK);box(tile,x-camera,150+FISH_NEAR_Y,4,14,RPG_WOOD);
        }
        box(tile,rail_left-camera,152+FISH_NEAR_Y,rail_right-rail_left,2,SHADOW);
        box(tile,rail_left-camera,150+FISH_NEAR_Y,rail_right-rail_left,2,FAR);
        box(tile,rail_left-camera,162+FISH_NEAR_Y,rail_right-rail_left,2,SHADOW);
        box(tile,rail_left-camera,160+FISH_NEAR_Y,rail_right-rail_left,2,FAR);
        arm_2d_point_float_t centre={FISH_STATION_X-camera,90+FISH_NEAR_Y};
        arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.station_operation,tile,NULL,&centre);
        ARM_2D_OP_WAIT_ASYNC(&scene.station_operation);
    }
    for (unsigned side=0;side<2;side++) {
        arm_2d_point_float_t centre={ (side?FISH_MAP_WIDTH-57:56)-camera,158+FISH_NEAR_Y };
        arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.dock_operation[side],tile,NULL,&centre);
        ARM_2D_OP_WAIT_ASYNC(&scene.dock_operation[side]);
        if(game->map_index)city_dock_steps(tile,camera,side);
        /* Pixel ripples meet the submerged pile ends; mirror the same authored
         * attachment points at the opposite bank. */
        static const int contacts[2][2][2]={{{35,25},{45,25}},{{10,43},{47,43}}};
        for(unsigned post=0;post<2;post++) {
            int local=contacts[game->map_index][post][0]-29;
            int px=(int)centre.fX+(side?-2:2)*local;
            int py=(int)centre.fY+2*(contacts[game->map_index][post][1]-25);
            box(tile,px-5,py-1,10,1,SHADOW);
            box(tile,px-7,py,5,1,RIPPLE);box(tile,px+3,py,5,1,RIPPLE);
        }
    }
    if(game->map_index) {
        const int lamps[]={-156,-20,1016,1150};
        for(unsigned i=0;i<4;i++) {
            int x=lamps[i]-camera;
            arm_2d_point_float_t centre={x,136+FISH_NEAR_Y};
            arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.park_lamp_operation,tile,NULL,&centre);
            ARM_2D_OP_WAIT_ASYNC(&scene.park_lamp_operation);
        }
    }
}

/* Emission is drawn after the night tint. Leave the foreground walker in front
 * of window light and canopy glow, including while passing the ticket office. */
static void station_light_patch(const arm_2d_tile_t *tile,const fish_game_t *g,
                                int x,int y,int w,int h,uint16_t color,unsigned alpha)
{
    y+=FISH_NEAR_Y;
    const int top=96+FISH_NEAR_Y,bottom=136+FISH_NEAR_Y;
    int actor=fish_game_actor_x(g)-fish_game_camera(g)-(g->facing<0),left=actor-14,right=actor+14;
    if(!fish_game_ashore(g) || x+w<=left || x>=right || y+h<=top || y>=bottom) {
        opacity_box(tile,x,y,w,h,color,alpha);return;
    }
    if(x<left)opacity_box(tile,x,y,left-x,h,color,alpha);
    if(x+w>right)opacity_box(tile,right,y,x+w-right,h,color,alpha);
    int a=x>left?x:left,b=x+w<right?x+w:right;
    if(y<top)opacity_box(tile,a,y,b-a,top-y,color,alpha);
    if(y+h>bottom)opacity_box(tile,a,bottom,b-a,y+h-bottom,color,alpha);
}

static void station_lights(const arm_2d_tile_t *tile,const fish_game_t *g)
{
    unsigned night=fish_game_night_opacity(g);int camera=fish_game_camera(g);
    if(g->map_index && (g->game_minute>=1080 || g->game_minute<360)) {
        const int lamps[]={-156,-20,1016,1150};
        for(unsigned i=0;i<4;i++) {
            int x=lamps[i]-camera;
            arm_2d_point_float_t centre={x+12,54+FISH_NEAR_Y};
            if(!fish_game_ashore(g)) {
                arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.park_light_operation,tile,NULL,&centre);
                ARM_2D_OP_WAIT_ASYNC(&scene.park_light_operation);
            } else {
                /* Clip the independent alpha texture around the foreground
                 * walker, keeping the sprite and ground shadow ahead of light. */
                int actor=fish_game_actor_x(g)-camera-(g->facing<0);
                int left=actor-14,right=actor+14;
                if(left<0)left=0;
                if(left>240)left=240;
                if(right<0)right=0;
                if(right>240)right=240;
                arm_2d_region_t regions[]={
                    {.tLocation={0,0},.tSize={240,96+FISH_NEAR_Y}},
                    {.tLocation={0,136+FISH_NEAR_Y},.tSize={240,104-FISH_NEAR_Y}},
                    {.tLocation={0,96+FISH_NEAR_Y},.tSize={left,40}},
                    {.tLocation={right,96+FISH_NEAR_Y},.tSize={240-right,40}}};
                for(unsigned part=0;part<4;part++)if(regions[part].tSize.iWidth>0) {
                    arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.park_light_operation,tile,&regions[part],&centre);
                    ARM_2D_OP_WAIT_ASYNC(&scene.park_light_operation);
                }
            }
        }
    }
    if(!night || camera<=FISH_MAP_WIDTH-280)return;
    int station=FISH_STATION_X-camera;unsigned strength=night*255/150;
    const uint16_t glow=RGB(248,177,80),core=RGB(255,236,177);
    const int lamps[]={-62,52};
    for(unsigned i=0;i<2;i++) {
        int x=station+lamps[i];
        station_light_patch(tile,g,x-10,90,20,12,glow,strength/12);
        station_light_patch(tile,g,x-6,92,12,4,RPG_WOOD,strength);
        station_light_patch(tile,g,x-4,96,8,2,core,strength);
        station_light_patch(tile,g,x-2,98,4,2,core,strength*3/4);
        /* Low-opacity, stepped light in the shelter; no full-screen overlay. */
        for(int y=100;y<134;y+=2) {
            int half=6+(y-100)/4*2;
            station_light_patch(tile,g,x-half,y,half*2,2,glow,strength/16);
        }
        station_light_patch(tile,g,x-26,136,52,2,glow,strength/8);
        station_light_patch(tile,g,x-20,138,40,2,glow,strength/7);
        station_light_patch(tile,g,x-12,140,24,2,glow,strength/10);
    }
    /* The ticket window remains lit all night; retain its mullion and frame. */
    station_light_patch(tile,g,station-48,108,4,14,glow,strength*3/4);
    station_light_patch(tile,g,station-42,108,2,14,core,strength*2/3);
}

static void shore_player(const arm_2d_tile_t *tile,const fish_game_t *game)
{
    int x=game->shore_x-fish_game_camera(game);
    int y=FISH_GROUND_Y;
    box(tile,x-7,y,14,2,SHADOW);
    arm_2d_point_float_t centre={x-(game->facing<0),y-16};
    arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&scene.shore_player_operation,tile,NULL,&centre);
    ARM_2D_OP_WAIT_ASYNC(&scene.shore_player_operation);
}
static void night_road(const arm_2d_tile_t *tile,const fish_game_t *g)
{
    unsigned night=fish_game_night_opacity(g);if(!night)return;
    int camera=fish_game_camera(g),left=24-camera,right=FISH_MAP_WIDTH-24-camera;
    if(left>240)left=240;if(right<0)right=0;
    unsigned alpha=g->map_index?night*3/4:night/3;
    int top=FISH_GROUND_Y+2;
    if(left>0)opacity_box(tile,0,top,left,204-top,RGB(5,12,24),alpha);
    if(right<240)opacity_box(tile,right,top,240-right,204-top,RGB(5,12,24),alpha);
}

static void water_splash(const arm_2d_tile_t *tile, const fish_game_t *game, int x, int y)
{
    int strength=fish_game_splash(game);
    unsigned elapsed=game->now-game->phase_at;
    int age=(int)(elapsed%480);
    int radius=5+age/40;
    box(tile,x-radius,y+4,radius*2,1,age<260?FOAM:RIPPLE);
    box(tile,x-radius-3,y+6,radius*2+6,1,RIPPLE);
    if (!strength) {
        if(game->mode==FISH_FIGHT && fish_game_burst_active(game) &&
           fish_species_profile(game->map_index,game->pending_species)->fight_style==FISH_FIGHT_SLOW) {
            /* Heavy bottom fish roll below the surface rather than leap. */
            box(tile,x-radius/2,y+2,radius,1,RIPPLE);
            box(tile,x-2,y+4,2,1,FOAM);box(tile,x+3,y+3,2,1,RIPPLE);
        }
        return;
    }
    /* Six tiny analytic droplets, staggered ages and seeded spread. No heap,
     * particle pool or bitmap; the same frozen time is used in every PFB block. */
    for (unsigned i=0;i<6;i++) {
        unsigned seed=game->pending_weight_g*17+i*79;
        int t=(int)((elapsed+seed%280)%360);
        if (t>280) continue;
        int sign=i&1?1:-1;
        int spread=2+(int)(seed%5);
        int dx=sign*(2+t*(spread+strength)/160);
        int height=(2+strength)*4*t*(280-t)/(280*280);
        box(tile,x+dx,y-height,1+(i%3==0),1+(t<100),t<160?CREAM:FOAM);
    }
    box(tile,x-3,y+1,6,1,FOAM);
}

/* City vessels sit8px lower; float flight, rod, ropes and splashes follow. */
static int water_y(const fish_game_t *game)
{return FISH_WATER_Y+(game->map_index?8:0);}
static bool float_pose(const fish_game_t *game,int *x,int *y,int *tip,int *body)
{
    if(fish_game_ashore(game) || game->mode<FISH_CAST || game->mode>FISH_FIGHT) return false;
        int floatx=game->cast_x-fish_game_camera(game), floaty=water_y(game);
        if (game->mode==FISH_CAST) {
            int t=(int)(game->now-game->phase_at); if (t>650) t=650;
            floatx=game->cast_start_x-fish_game_camera(game)+(game->cast_x-game->cast_start_x)*t/650;
            floaty=water_y(game)-23+23*t/650-4*44*t*(650-t)/(650*650);
        } else {
            int dx,dy; fish_game_float(game,&dx,&dy);
            floatx+=dx*game->facing; floaty+=dy;
        }
        const int surface_y=water_y(game);
        const fish_float_t *float_art=&fish_floats[game->float_index];
        int red_height=float_art->tip_height,cream_height=float_art->body_height;
        if (game->mode!=FISH_CAST) {
            red_height=surface_y+1-(floaty-float_art->tip_height); if (red_height>float_art->tip_height) red_height=float_art->tip_height;
            cream_height=surface_y+1-floaty; if (cream_height>float_art->body_height) cream_height=float_art->body_height;
        }
    *x=floatx;*y=floaty;*tip=red_height>0?red_height:0;*body=cream_height>0?cream_height:0;
    return true;
}

static void night_float(const arm_2d_tile_t *tile,const fish_game_t *game)
{
    unsigned night=fish_game_night_opacity(game);
    if(!night || !fish_floats[game->float_index].night_bite_bonus_percent) return;
    int x,y,tip,body;
    if(!float_pose(game,&x,&y,&tip,&body) || (!tip && !body)) return;
    int top=y-fish_floats[game->float_index].tip_height;
    int height=tip+(body>0?body:0);
    unsigned opacity=night*255/150;
    arm_2d_region_t halo={.tLocation={x-4,top-1},.tSize={8,height+2}};
    if(game->mode!=FISH_CAST && halo.tLocation.iY+halo.tSize.iHeight>water_y(game)+1)
        halo.tSize.iHeight=water_y(game)+1-halo.tLocation.iY;
    arm_2d_rgb565_fill_colour_with_opacity(tile,&halo,
        (arm_2d_color_rgb565_t){.tValue=RGB(26,153,133)},(uint8_t)(opacity/4));
    ARM_2D_OP_WAIT_ASYNC();
    arm_2d_region_t core={.tLocation={x-2,top},.tSize={4,tip}};
    if(tip) {
        arm_2d_rgb565_fill_colour_with_opacity(tile,&core,
            (arm_2d_color_rgb565_t){.tValue=RGB(76,255,218)},(uint8_t)opacity);ARM_2D_OP_WAIT_ASYNC();
    }
    if(body) {
        core=(arm_2d_region_t){.tLocation={x-2,y},.tSize={4,body}};
        arm_2d_rgb565_fill_colour_with_opacity(tile,&core,
            (arm_2d_color_rgb565_t){.tValue=RGB(43,207,190)},(uint8_t)opacity);ARM_2D_OP_WAIT_ASYNC();
    }
}

static void boat_water_contact(const arm_2d_tile_t *tile,int x,int y)
{
    /* A shallow water overlap at the hull, with no detached drop shadow. */
    opacity_box(tile,x-21,y+12,42,2,SHADOW,112);
    box(tile,x-23,y+13,7,1,RIPPLE);box(tile,x+16,y+13,7,1,RIPPLE);
    box(tile,x-17,y+15,12,1,RIPPLE);box(tile,x+7,y+16,10,1,RIPPLE);
}
static void fisherman(const arm_2d_tile_t *tile, const fish_game_t *game)
{
    int x=game->boat_x-fish_game_camera(game);
    int bob=(game->now/450)%4; bob=bob==3?1:bob;
    int y=water_y(game)+bob;
    if (fish_game_ashore(game)) {
        arm_2d_region_t hull_region={.tLocation={0,25},.tSize={64,11}};
        arm_2d_tile_t hull,mask;
        arm_2d_tile_generate_child(&c_tileBoatRGB565,&hull_region,&hull,false);
        arm_2d_tile_generate_child(&c_tileBoatAlphaGRAY8,&hull_region,&mask,false);
        masked_bitmap(tile,&hull,&mask,x-32,y+3,game->boat_x>FISH_MAP_WIDTH/2);
        boat_water_contact(tile,x,y);
        shore_player(tile,game);
        return;
    }
    masked_bitmap(tile,&c_tileBoatRGB565,&c_tileBoatAlphaGRAY8,x-32,y-22,game->facing<0);
    boat_water_contact(tile,x,y);
    int rodx=x+game->facing*10, rody=y-7;
    int tipx=x+game->facing*(game->mode==FISH_CHARGE ? -12 : 28);
    int tipy=y-(game->mode==FISH_CHARGE ? 43 : 35);
    if (fish_game_burst_warning(game)) tipy-=2;
    unsigned loss_age=game->now-game->phase_at;
    if(game->mode==FISH_LOST && game->loss==FISH_LOSS_BREAK && loss_age<400)
        tipy+=(400-(int)loss_age)*((loss_age/70)%2?1:-1)/100;
    unsigned flex=fish_game_rod_flex(game);
    if(flex) {
        tipx+=game->facing*(int)flex/10;tipy+=(int)flex/6;
        int cx=(rodx+tipx)/2-game->facing*(int)flex/16,cy=(rody+tipy)/2-(int)flex/7;
        int px=rodx,py=rody;
        for(int step=1;step<=8;step++) {
            int a=8-step,nx=(a*a*rodx+2*a*step*cx+step*step*tipx)/64;
            int ny=(a*a*rody+2*a*step*cy+step*step*tipy)/64;
            line(tile,px,py,nx,ny,INK);line(tile,px+game->facing,py,nx+game->facing,ny,WOOD_LIGHT);
            px=nx;py=ny;
        }
    } else {
        line(tile,rodx,rody,tipx,tipy,INK);
        line(tile,rodx+game->facing,rody,tipx+game->facing,tipy,WOOD_LIGHT);
    }
    if(game->mode==FISH_LOST) {
        int t=(int)loss_age, camera=fish_game_camera(game);
        int fx=game->cast_x-camera+game->float_dx_q8/256*game->facing;
        int fy=water_y(game)+game->float_dy_q8/256;
        int radius=3+t/45;
        if(t<900) {
            box(tile,fx-radius,water_y(game)+4,radius*2,1,t<450?FOAM:RIPPLE);
            box(tile,fx-radius/2,water_y(game)+6,radius,1,RIPPLE);
        }
        if(game->loss==FISH_LOSS_BREAK) {
            int q=t<550?t:550;
            int ex=(tipx+fx)/2,ey=(tipy+water_y(game))/2;
            int rx=ex+(tipx+game->facing*8-ex)*q/550;
            int ry=ey+(tipy+12-ey)*q/550;
            line(tile,tipx,tipy,rx,ry,CREAM);
            if(t<500) {
                line(tile,fx,water_y(game),ex+(t/80)*game->facing,ey+t/18,FOAM);
                box(tile,ex-3-t/60,ey-2,2,1,SUN);box(tile,ex+3+t/60,ey+2,2,1,SUN);
            }
            fy+=t/45;
        } else if(game->loss==FISH_LOSS_MISSED || game->loss==FISH_LOSS_RANGE) {
            fy+=t/35;
            if(t<320) line(tile,tipx,tipy,fx,water_y(game),CREAM);
            else line(tile,tipx,tipy,tipx+game->facing*8,tipy+12,CREAM);
        } else {
            int q=t<900?t:900;
            fx+=(tipx+game->facing*6-fx)*q/900;
            fy+=(tipy+18-fy)*q/900;
            int mx=(tipx+fx)/2,my=(tipy+fy)/2+12*(900-q)/900;
            line(tile,tipx,tipy,mx,my,CREAM);line(tile,mx,my,fx,fy,CREAM);
            /* The empty hook becomes visible only after the rig leaves water. */
            if(fy<water_y(game)-2) {box(tile,fx,fy+7,1,4,FOAM);box(tile,fx-2,fy+10,3,1,FOAM);}
        }
        const fish_float_t *art=&fish_floats[game->float_index];
        int top=fy-art->tip_height;
        if(top<water_y(game)+1) {
            int h=water_y(game)+1-top;if(h>art->tip_height) h=art->tip_height;
            box(tile,fx-1,top,3,h,art->tip);
            if(fy<water_y(game)) box(tile,fx-1,fy,3,art->body_height,art->body);
        }
        return;
    }
    if (game->mode>=FISH_CAST && game->mode<=FISH_FIGHT) {
        int floatx,floaty,red_height,cream_height;
        float_pose(game,&floatx,&floaty,&red_height,&cream_height);
        line(tile,tipx,tipy,(tipx+floatx)/2,tipy+18,CREAM);
        int surface_y=water_y(game);
        int visible_y=game->mode==FISH_CAST?floaty:floaty>surface_y?surface_y:floaty;
        line(tile,(tipx+floatx)/2,tipy+18,floatx,visible_y,CREAM);
        box(tile,floatx-2,floaty-fish_floats[game->float_index].tip_height,4,red_height,fish_floats[game->float_index].tip);
        box(tile,floatx-2,floaty,4,cream_height,fish_floats[game->float_index].body);
        if (game->mode!=FISH_CAST) {
            water_splash(tile,game,floatx,surface_y);
        }
        if (game->mode==FISH_BITE) {
            box(tile,floatx-2, floaty-22,4,10,SUN); box(tile,floatx-2,floaty-9,4,3,SUN);
        }
    }
}

static void chinese_text(const arm_2d_tile_t *tile, int centre_x, int y,
                         const char *value, arm_2d_size_t size);
static void chinese_text_tinted(const arm_2d_tile_t *tile,int centre_x,int y,const char *value,
                               arm_2d_size_t size,uint16_t color,uint16_t background);

static const char *control_hint(const fish_game_t *game)
{
    if(game->mode==FISH_SHORE && fish_game_at_station(game)) return "短按进入车站";
    if(game->mode==FISH_SHOP) return "旋转选商品，短按交易";
    if(game->mode==FISH_HOLD) return "旋转查看鱼获，短按返回";
    if(game->mode==FISH_SHORE && fish_game_at_shop(game)) return fish_game_shop_open(game)?(game->map_index?(abs(game->shore_x-FISH_CITY_STALL_X)<=FISH_SHOP_REACH?"短按进入江边鱼摊":"短按进入江边便利店"):"短按进入水产渔具店"):(game->map_index?(abs(game->shore_x-FISH_CITY_STALL_X)<=FISH_SHOP_REACH?"营业06:00-16:00":"营业09:00-21:00"):"营业07:00-18:00");
    const char *hint="";
    switch (game->mode) {
        case FISH_AIM: break;
        case FISH_CHARGE: hint="松开抛竿"; break;
        case FISH_CAST: hint="稍等片刻"; break;
        case FISH_WAIT: hint="等鱼咬钩  短按收回"; break;
        case FISH_BITE: hint=fish_game_strike_ready(game)?"鱼咬实了！短按提竿":"还在试饵，稍等再提竿"; break;
        case FISH_FIGHT: hint="顺转收线  逆转放线"; break;
        case FISH_CAUGHT: hint="又有收获！"; break;
        case FISH_GUIDE: hint="旋转选鱼 短按翻页 长按返回"; break;
        case FISH_SHORE: hint="旋转走动  短按打开背包"; break;
        case FISH_BAG:
            hint=game->bag_editing?"旋转调整  短按确认":"旋转选格子  短按选配件";
            if(game->bag_index==FISH_BAG_SOUND && !game->bag_editing)hint="旋转选格子 短按设置声音";
            if(game->bag_index==FISH_BAG_SAVE) hint="";
            break;
        default: break;
    }
    if(game->mode==FISH_LOST)
        hint=game->loss==FISH_LOSS_BREAK?"线组已损坏":
             game->loss==FISH_LOSS_SLACK?"脱钩了":
             game->loss==FISH_LOSS_EARLY?"提早了":
             game->loss==FISH_LOSS_MISSED?"错过了":"鱼逃走了";
    if (game->mode==FISH_AIM && game->loss!=FISH_LOSS_NONE)
        hint=game->loss==FISH_LOSS_BREAK?"线组已损坏":
             game->loss==FISH_LOSS_SLACK?"脱钩了":
             game->loss==FISH_LOSS_EARLY?"提早了":
             game->loss==FISH_LOSS_MISSED?"错过了":"鱼逃走了";
    if (game->mode==FISH_FIGHT) {
        if(fish_game_distance(game)>=game->line_capacity_units) hint="已到最大线长，无法继续放线";
        else if (game->tension*100/game->line_strength>=85 || game->overload_ms>=fish_rules.overload_limit_ms/3) hint="张力过高！停止收线，逆转放线";
        else if (game->slack_ms>=fish_rules.slack_limit_ms/4) hint="线太松了！顺转收线";
        else if (fish_game_burst_active(game) && game->tension*100/game->line_strength>=65)
            hint=fish_species[game->pending_species].fight_style==FISH_FIGHT_ACTIVE?"鱼在猛拉！逆转放线":"线在绷紧！留意张力";
        else if (fish_game_distance(game)*100/game->line_capacity_units>=fish_rules.range_warning_percent) hint="鱼逃远了！顺转收线";
        else if (fish_game_burst_warning(game))
            hint=fish_species[game->pending_species].fight_style==FISH_FIGHT_ACTIVE?"准备挣扎！留意张力":"缓慢拉线，留意张力";
    }
    if (game->mode==FISH_WAIT && game->pending_species>=FISH_SPECIES_COUNT && game->now-game->phase_at>=2600)
        hint="这里暂时没鱼，短按收回";
    if((game->mode==FISH_MOVE || game->mode==FISH_AIM) && !game->economy.lines)hint="线组已损坏，去商店更换";
    return hint;
}

static void position_and_depth(const arm_2d_tile_t *tile,const fish_game_t *game)
{
    arm_2d_region_t region={.tLocation={0,17},.tSize={240,24}};
    if (!arm_2d_helper_pfb_is_region_active(tile,&region,true)) return;
    int actor=fish_game_actor_x(game),marker=12+actor*215/FISH_MAP_WIDTH;
    if(marker<12) marker=12;
    if(marker>227) marker=227;
    box(tile,12,19,216,3,DARK_PINE);
    for (unsigned i=0;i<2;i++) box(tile,12+fish_docks[i].boat_x*215/FISH_MAP_WIDTH,18,1,5,WOOD_LIGHT);
    box(tile,marker-1,17,3,7,SUN);
    for (unsigned i=1;i<FISH_DEPTH_NODE_COUNT;i++) {
        const fish_depth_node_t *profile=game->map_index?fish_city_depth_profile:fish_depth_profile;
        int maximum=game->map_index?FISH_CITY_MAX_DEPTH_CM:FISH_MAX_DEPTH_CM;
        const fish_depth_node_t *a=&profile[i-1],*b=&profile[i];
        line(tile,12+a->x*215/FISH_MAP_WIDTH,27+a->depth_cm*11/maximum,
            12+b->x*215/FISH_MAP_WIDTH,27+b->depth_cm*11/maximum,RIPPLE);
    }
    int depth=fish_map_depth_on(game->map_index,actor),maximum=game->map_index?FISH_CITY_MAX_DEPTH_CM:FISH_MAX_DEPTH_CM;
    box(tile,marker-1,26+depth*11/maximum,3,3,SUN);
    if (game->mode>=FISH_CAST && game->mode<=FISH_FIGHT) {
        int x=12+game->cast_x*215/FISH_MAP_WIDTH;
        box(tile,x,24,1,4+game->cast_bottom_cm*11/maximum,WOOD_LIGHT);
        box(tile,x-1,26+game->cast_depth_cm*11/maximum,3,2,CREAM);
    }
}

static void coin_count(const arm_2d_tile_t *tile,int x,int y,uint32_t coins)
{
    box(tile,x+2,y,5,1,RPG_BRASS);box(tile,x,y+2,9,5,RPG_BRASS);
    box(tile,x+2,y+8,5,1,RPG_EDGE);box(tile,x+2,y+1,5,6,SUN);
    box(tile,x+4,y+2,1,4,WOOD);box(tile,x+2,y+2,1,1,CREAM);
    char amount[16];snprintf(amount,sizeof(amount),"%lu",(unsigned long)coins);
    text(tile,x+14,y+1,SUN,amount);
}

/* Compact buff/debuff sockets: only the18px icon footprint is tinted. The
 * bottom bright area is the current value; the upper area shades the icon. */
static void status_icon(const arm_2d_tile_t *tile,int x,int y,unsigned value,unsigned full,bool latte)
{
    if(value>full)value=full;
    unsigned fill=(value*16+full-1)/full;
    opacity_box(tile,x,y,18,18,RGB(17,27,34),96);
    uint16_t color=latte?RGB(147,102,47):RGB(41,123,151);
    if(fill)opacity_box(tile,x+1,y+17-(int)fill,16,(int)fill,color,192);
    if(latte) {
        const arm_2d_tile_t *icon=fish_shop_list_goods[FISH_SHOP_LATTE];
        arm_2d_region_t region={.tLocation={x+1,y+1},.tSize={16,16}};
        arm_2d_rgb16_tile_copy_with_colour_keying(icon,tile,&region,icon->phwBuffer[0],ARM_2D_CP_MODE_COPY);
        ARM_2D_OP_WAIT_ASYNC();
    } else {
        static const uint8_t drop[10]={0x10,0x18,0x38,0x7c,0x7c,0xfe,0xfe,0xfe,0x7c,0x38};
        for(unsigned row=0;row<10;row++)for(unsigned col=0;col<8;col++)
            if(drop[row]&(1u<<col))box(tile,x+5+(int)col,y+3+(int)row,1,1,RGB(166,216,232));
        box(tile,x+8,y+7,1,3,RGB(225,241,239));
    }
    if(fill<16)opacity_box(tile,x+1,y+1,16,16-(int)fill,RGB(17,27,34),152);
}

static void hud(const arm_2d_tile_t *tile, const fish_scene_t *self)
{
    const fish_game_t *game=&self->game;
    char label[48];
    bool map_hud=game->mode!=FISH_BAG && game->mode!=FISH_GUIDE && game->mode!=FISH_CAUGHT &&
        game->mode!=FISH_SHOP && game->mode!=FISH_HOLD;
    int header_height=map_hud?42:28;
    box(tile,0,0,240,header_height,game->mode==FISH_BAG?RPG_DARK:INK);
    if (game->mode==FISH_BAG) {
        const fish_ui_art_t *art=&fish_equipment_art[RPG_SATCHEL_ART];
        masked_bitmap(tile,art->sprite,art->mask,10,4,false);
        chinese_text_tinted(tile,53,1,"行囊",(arm_2d_size_t){26,19},RPG_LIGHT,RPG_DARK);
        coin_count(tile,83,8,game->economy.coins);
    } else if(game->mode!=FISH_SHOP && game->mode!=FISH_HOLD) coin_count(tile,8,7,game->economy.coins);
    if(game->mode!=FISH_BAG && game->mode!=FISH_GUIDE && game->mode!=FISH_SHOP && game->mode!=FISH_HOLD) {
        snprintf(label,sizeof(label),"%02u:%02u",game->game_minute/60,game->game_minute%60);text(tile,121,7,CREAM,label);
    }
    if (game->mode==FISH_GUIDE) {
        ui_text(tile,122,4,48,GREEN,INK,"图鉴");
        snprintf(label,sizeof(label),"%02u/%02u",fish_guide_position(game->guide_index)+1,FISH_GUIDE_COUNT);
        text(tile,177,7,GREEN,label);
        text(tile,216,7,CREAM,game->guide_page?"2/2":"1/2");
    } else if (map_hud) {
        int depth=fish_map_depth_on(game->map_index,fish_game_actor_x(game));
        snprintf(label,sizeof(label),"水深%d.%d米",depth/100,depth%100/10);
        ui_text(tile,166,0,74,GREEN,INK,label);
    }
    if (map_hud) position_and_depth(tile,game);
    if(map_hud || game->mode==FISH_BAG) {
        int x=game->mode==FISH_BAG?180:8,y=game->mode==FISH_BAG?5:44;
        if(game->wetness_units || fish_weather_rain(game)) {
            status_icon(tile,x,y,game->wetness_units,FISH_WETNESS_FULL,false);x+=22;
        }
        if(game->latte_remaining_ms)status_icon(tile,x,y,game->latte_remaining_ms,FISH_LATTE_DURATION_MS,true);
    }
    box(tile,0,header_height-1,240,1,game->mode==FISH_BAG?RPG_BRASS:HILL);
    box(tile,0,203,240,37,game->mode==FISH_BAG?RPG_DARK:INK);
    box(tile,0,203,240,1,game->mode==FISH_BAG?RPG_EDGE:HILL);
    static const char *modes[]={"划船","垂钓","蓄力","抛竿","等待","咬钩","搏鱼","收获","图鉴","行囊","岸上","跑鱼","商店","鱼库"};
    const char *mode=modes[game->mode], *hint=control_hint(game);
    if ((game->mode==FISH_MOVE || game->mode==FISH_AIM) && game->boat_speed)
        mode=game->boat_speed==1 || game->boat_speed==-1?"慢行":"快行";
    if(game->mode==FISH_SHORE && game->shore_speed) mode="步行";
    if(game->mode==FISH_SHORE && fish_game_at_station(game)) mode="车站";
    if (game->mode==FISH_BITE && !fish_game_strike_ready(game)) mode="试饵";
    ui_text(tile,9,204,52,game->mode==FISH_LOST?RED:game->mode==FISH_BITE?SUN:GREEN,INK,mode);
    snprintf(label,sizeof(label),"鱼获%d",game->catches); ui_text(tile,180,204,60,CREAM,INK,label);
    chinese_text(tile,120,220,hint,self->hint_box);
    if (game->mode==FISH_GUIDE) {
        unsigned found=0;for(unsigned i=0;i<FISH_GUIDE_COUNT;i++)found+=game->species_catches[fish_guide_index_at(i)]>0;
        snprintf(label,sizeof(label),"收集%u/%u",found,FISH_GUIDE_COUNT);
        ui_text(tile,64,204,104,fish_rarity_colors[fish_species[game->guide_index].rarity],INK,label);
    }
    if (game->mode==FISH_MOVE || game->mode==FISH_AIM) {
        uint16_t depth=fish_game_rig_depth(game);
        if (depth==FISH_BOTTOM_OPTION) snprintf(label,sizeof(label),"钓底");
        else snprintf(label,sizeof(label),"钓深%u.%u米",depth/100,depth%100/10);
        ui_text(tile,66,204,92,HILL,INK,label);
    }
    if (game->mode==FISH_CHARGE) {
        int power=fish_game_power(game);
        box(tile,64,210,88,10,SHADOW);
        box(tile,66,212,power*84/100,6,power>=85?SUN:GREEN);
        snprintf(label,sizeof(label),"%d%%",power); text(tile,158,211,CREAM,label);
        if (game->mode==FISH_CHARGE) {
            int x=game->boat_x-fish_game_camera(game), d=28+power*(fish_game_max_cast_px(game)-28)/100;
            int target=x+game->facing*(12+d);
            box(tile,target-3,148+FISH_NEAR_Y,7,1,SUN); box(tile,target,145+FISH_NEAR_Y,1,7,SUN);
        }
    }
    if (game->mode==FISH_FIGHT) {
        box(tile,5,183,230,17,INK);
        ui_text(tile,8,183,32,CREAM,INK,"张力");
        int stamina=game->stamina_max?game->stamina*100/game->stamina_max:0;
        box(tile,64,210,88,10,SHADOW);box(tile,66,212,stamina*84/100,6,stamina>50?GREEN:SUN);
        snprintf(label,sizeof(label),"%d%%",stamina);text(tile,158,211,CREAM,label);
        box(tile,40,187,102,8,INK); box(tile,41,188,100,6,SHADOW);
        int tension=game->tension*100/game->line_strength; if (tension>100) tension=100;
        box(tile,41,188,tension,6,tension>=85?RED:tension<=12?SUN:GREEN);
        box(tile,126,187,1,8,CREAM);
        box(tile,144,188,3,6,game->overload_ms?RED:game->slack_ms?SUN:HILL);
        int range=fish_game_distance(game)*100/game->line_capacity_units;
        unsigned decimetres=(fish_game_distance_cm(game)+9)/10;
        snprintf(label,sizeof(label),"距离%u.%u米",decimetres/10,decimetres%10);
        ui_text(tile,152,183,88,range>=fish_rules.range_warning_percent?RED:CREAM,INK,label);
    }
}

static void chinese_text(const arm_2d_tile_t *tile, int centre_x, int y, const char *value, arm_2d_size_t size)
{
    chinese_text_tinted(tile,centre_x,y,value,size,CREAM,INK);
}

static void chinese_text_tinted(const arm_2d_tile_t *tile,int centre_x,int y,const char *value,
                               arm_2d_size_t size,uint16_t color,uint16_t background)
{
    arm_2d_region_t region={.tLocation={centre_x-size.iWidth/2,y},.tSize=size};
    arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)tile);
    arm_lcd_text_set_font(fish_remarks_font());
    arm_lcd_text_set_draw_region(&region);
    arm_lcd_text_set_colour(color,background);
    arm_lcd_text_location(0,0);
    arm_lcd_puts(value);
    ARM_2D_OP_WAIT_ASYNC();
}

static const char *bag_value(const fish_game_t *g,unsigned item,char *scratch,size_t length)
{
    switch (item) {
        case FISH_BAG_ROD:return fish_rods[g->rod_index].name;
        case FISH_BAG_LINE: {
            if(!g->economy.lines)return "线组已损坏";
            unsigned cm=fish_line_sets[g->line_index].distance_limit_cm;
            snprintf(scratch,length,"%s %u.%u米",fish_line_sets[g->line_index].name,cm/100,cm%100/10);return scratch;
        }
        case FISH_BAG_FLOAT:return fish_floats[g->float_index].name;
        case FISH_BAG_BAIT:
            if(g->bait_index==FISH_PLASTIC_BAIT) return fish_baits[g->bait_index].name;
            snprintf(scratch,length,"%s x%u",fish_baits[g->bait_index].name,g->economy.bait_stock[g->bait_index]);return scratch;
        case FISH_BAG_DEPTH: {
            uint16_t depth=fish_game_rig_depth(g);
            if (depth==FISH_BOTTOM_OPTION) return "到底";
            if (!depth) return "表层";
            snprintf(scratch,length,"%u.%u米",depth/100,depth%100/10);return scratch;
        }
        case FISH_BAG_MISC:return fish_containers[g->economy.container].name;
        case FISH_BAG_GUIDE:return "图鉴";
        case FISH_BAG_SAVE:return "存档";
        case FISH_BAG_SOUND:return "声音";
        default:return "返回";
    }
}

static void rpg_slot(const arm_2d_tile_t *tile,int x,int y,int w,int h,bool selected,bool editing)
{
    uint16_t edge=selected?(editing?SUN:RPG_BRASS):RPG_EDGE;
    if(!selected) return;
    for(int dx=5;dx<w-5;dx+=5) {box(tile,x+dx,y,2,1,edge);box(tile,x+dx,y+h-1,2,1,edge);}
    for(int dy=5;dy<h-5;dy+=5) {box(tile,x,y+dy,1,2,edge);box(tile,x+w-1,y+dy,1,2,edge);}
    if (selected) {
        uint16_t accent=editing?SUN:RPG_LIGHT;
        box(tile,x-1,y-1,6,2,accent); box(tile,x-1,y-1,2,6,accent);
        box(tile,x+w-5,y+h-1,6,2,accent); box(tile,x+w-1,y+h-5,2,6,accent);
        box(tile,x+5,y+5,3,3,accent);
    }
}

static const char *bag_attributes(const fish_game_t *g,char *scratch,size_t length)
{
    switch (g->bag_index) {
        case FISH_BAG_ROD:return fish_rod_descriptions[g->rod_index];
        case FISH_BAG_LINE:
        {
            if(!g->economy.lines)return "去商店更换线组";
            if(g->line_index==FISH_PE_LINE)return "像一条钢缆，会吓跑小鱼";
            const fish_line_set_t *line=&fish_line_sets[g->line_index];
            snprintf(scratch,length,"0.%02u毫米 隐蔽%s",line->diameter_hundredths_mm,
                line->bite_percent[0]>=90?"高":line->bite_percent[0]>=60?"中":"低");return scratch;
        }
        case FISH_BAG_FLOAT:return fish_float_descriptions[g->float_index];
        case FISH_BAG_BAIT:return fish_baits[g->bait_index].description;
        case FISH_BAG_MISC:
            snprintf(scratch,length,"容量%lu kg",(unsigned long)fish_hold_capacity(&g->economy)/1000);return scratch;
        case FISH_BAG_DEPTH:return fish_game_rig_depth(g)==FISH_BOTTOM_OPTION?"随当地水底调整":"距水面的下饵深度";
        case FISH_BAG_GUIDE:return "查看已收集的鱼种";
        case FISH_BAG_SAVE:return "每五分钟自动存档";
        case FISH_BAG_SOUND:return fish_sound_names[g->sound_mode];
        default:return "回到湖上继续垂钓";
    }
}

static void bag_panel(const arm_2d_tile_t *tile,const fish_scene_t *self)
{
    const fish_game_t *g=&self->game;
    if (g->mode!=FISH_BAG) return;
    arm_2d_region_t region={.tLocation={4,29},.tSize={232,172}};
    if (!arm_2d_helper_pfb_is_region_active(tile,&region,true)) return;
    /* Fixed sockets, one shared detail strip: names never compete with sprites. */
    static const int slots[FISH_BAG_COUNT][4]={
        {30,58,84,82},{121,62,40,30},{173,62,40,30},{121,108,40,32},
        {173,108,40,32},{26,146,46,31},{0,0,0,0},{6,183,54,18},{180,183,54,18},{122,183,54,18},{64,183,54,18}};
    static const char *titles[]={"鱼竿","线钩","浮漂","鱼饵","钓深"};
    char label[64],scratch[48];
    arm_2d_point_float_t centre={120.0f,115.0f};
    arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&self->backpack_operation,tile,NULL,&centre);
    ARM_2D_OP_WAIT_ASYNC(&self->backpack_operation);
    for(unsigned i=0;i<FISH_BAG_COUNT;i++) {
        if(i==FISH_BAG_DOCK) continue;
        int x=slots[i][0],y=slots[i][1],w=slots[i][2],h=slots[i][3];
        rpg_slot(tile,x,y,w,h,i==g->bag_index,g->bag_editing);
        if(i>FISH_BAG_ROD && i<=FISH_BAG_DEPTH) {
            unsigned art=i==FISH_BAG_LINE?RPG_LINE_ART+(g->line_index==FISH_PE_LINE?2:g->line_index):
                i==FISH_BAG_FLOAT?RPG_FLOAT_ART+g->float_index:
                i==FISH_BAG_BAIT?RPG_BAIT_ART+g->bait_index:RPG_DEPTH_ART;
            if(i==FISH_BAG_BAIT && g->bait_index==FISH_PLASTIC_BAIT) {
                box(tile,x+11,y+6,16,4,RPG_PAPER);box(tile,x+13,y+5,10,1,RPG_LIGHT);
                box(tile,x+24,y+6,2,2,RED);line(tile,x+13,y+10,x+14,y+14,RPG_BRASS);
                line(tile,x+14,y+14,x+17,y+12,RPG_BRASS);
            } else if(i==FISH_BAG_BAIT && g->bait_index>=FISH_BAIT_SNAIL) {
                arm_2d_region_t icon={.tLocation={x+12,y+3},.tSize={16,16}};
                arm_2d_rgb16_tile_copy(fish_shop_list_goods[FISH_SHOP_SNAIL+g->bait_index-FISH_BAIT_SNAIL],tile,&icon,ARM_2D_CP_MODE_COPY);ARM_2D_OP_WAIT_ASYNC();
            } else masked_bitmap(tile,fish_equipment_art[art].sprite,fish_equipment_art[art].mask,x+10,y+1,false);
            chinese_text_tinted(tile,x+w/2,y+h-13,titles[i],(arm_2d_size_t){26,19},RPG_LIGHT,RPG_DARK);
        } else if(i==FISH_BAG_MISC) {
            const fish_ui_art_t *art=&fish_container_art[g->economy.container];
            masked_bitmap(tile,art->sprite,art->mask,x+7,y-3,false);
            snprintf(label,sizeof(label),"%lu.%lu",(unsigned long)fish_hold_weight(&g->economy)/1000,
                (unsigned long)fish_hold_weight(&g->economy)%1000/100);text(tile,x+7,y+17,RPG_BRASS,label);
        } else if(i>=FISH_BAG_DOCK) {
            const char *value=bag_value(g,i,scratch,sizeof(scratch));
            uint16_t ink=i==FISH_BAG_DOCK && fish_game_dock_action(g)<0?RPG_MUTED:RPG_LIGHT;
            chinese_text_tinted(tile,x+w/2,y-3,value,self->bag_names[i],ink,RPG_DARK);
        }
    }
    chinese_text_tinted(tile,72,42,fish_rods[g->rod_index].name,self->bag_names[FISH_BAG_ROD],RPG_LIGHT,RPG_DARK);
    const fish_ui_art_t *rod=&fish_equipment_art[g->rod_index];
    scaled_species_bitmap(tile,rod->sprite,rod->mask,72,101,false,512,
        (arm_2d_op_trans_msk_t *)&self->portrait_operation);
    box(tile,76,146,156,33,RPG_DARK);
    const char *value=bag_value(g,g->bag_index,scratch,sizeof(scratch));
    chinese_text_tinted(tile,78+self->bag_names[g->bag_index].iWidth/2,145,value,
        self->bag_names[g->bag_index],RPG_LIGHT,RPG_DARK);
    const char *attributes=bag_attributes(g,label,sizeof(label));
    chinese_text_tinted(tile,78+self->guide_name_box.iWidth/2,162,attributes,self->guide_name_box,RPG_LIGHT,RPG_DARK);
}

static void caption(const arm_2d_tile_t *tile,int x,int y,const char *label,uint16_t ink,uint16_t bg)
{
    arm_2d_size_t size=arm_lcd_get_string_line_box(label,fish_remarks_font());
    if(fish_smoke_test && size.iWidth>226) {fprintf(stderr,"Caption exceeds screen: %s\n",label);exit(1);}
    chinese_text_tinted(tile,x,y,label,size,ink,bg);
}
static void shop_interior(const arm_2d_tile_t *tile,unsigned first_row)
{
    if(scene.game.map_index && scene.game.shop_venue==FISH_VENDOR_TACKLE) {
        vendor_image(tile,&fish_image_CityStallInterior,0,54,first_row);return;
    }
    /* Reuse the night layer's two scanlines; each copy finishes before reuse. */
    for(unsigned y=first_row;y<72;y++) {
        /* Match the original Arm-2D transform's 238-pixel footprint exactly. */
        arm_2d_region_t row={.tLocation={0,54+(int)y*2},.tSize={238,2}};
        if(!arm_2d_helper_pfb_is_region_active(tile,&row,true))continue;
        for(unsigned x=0;x<120;x++) {
            uint16_t color=fish_shop_interior_pixel(x,y);
            city_night_rows[x*2]=city_night_rows[x*2+1]=color;
            city_night_rows[240+x*2]=city_night_rows[241+x*2]=color;
        }
        arm_2d_rgb16_tile_copy(&city_night_row_tile,tile,&row,ARM_2D_CP_MODE_COPY);
        ARM_2D_OP_WAIT_ASYNC();
    }
}
static void shop_panel(const arm_2d_tile_t *tile,const fish_scene_t *self)
{
    const fish_game_t *g=&self->game;if(g->mode!=FISH_SHOP) return;
    shop_interior(tile,0);
    if(g->map_index && g->shop_venue==FISH_VENDOR_CONVENIENCE)
        vendor_image(tile,g->merchant_reply==FISH_MERCHANT_LATTE?&fish_image_CityClerkLatte:g->merchant_reply==FISH_MERCHANT_POOR?&fish_image_CityClerkConcerned:&fish_image_CityClerkGreeting,128,56,0);
    else {
        static const fish_vendor_image_t *const portraits[]={&fish_image_MerchantGreeting,&fish_image_MerchantSold,&fish_image_MerchantBought,&fish_image_MerchantSurprised};
        vendor_image(tile,portraits[fish_merchant_pose(g)%4],128,56,0);
    }
    /* Restore the original near counter edge IN FRONT of the seller. The
     * child tile reuses the same image pixels; no duplicate bitmap. */
    shop_interior(tile,50);
    box(tile,0,0,240,28,RPG_DARK);caption(tile,55,3,g->map_index?(g->shop_venue==FISH_VENDOR_CONVENIENCE?"江边便利店":"江边鱼摊"):"水产渔具店",RPG_LIGHT,RPG_DARK);
    char label[48];coin_count(tile,128,9,g->economy.coins);
    unsigned item=g->shop_index,kind=fish_shop_kind(item),index=fish_shop_item_index(item);
    unsigned count=fish_shop_count(g),position=fish_shop_position(g,item),first=position>2?position-2:0;
    if(count>FISH_SHOP_LIST_ROWS && first>count-FISH_SHOP_LIST_ROWS) first=count-FISH_SHOP_LIST_ROWS;
    box(tile,FISH_SHOP_LIST_X,FISH_SHOP_LIST_Y,FISH_SHOP_LIST_W,FISH_SHOP_LIST_H,RPG_DARK);
    for(unsigned row=0;row<FISH_SHOP_LIST_ROWS && first+row<count;row++) {
        unsigned id=fish_shop_item_at(g,first+row);int y=FISH_SHOP_LIST_Y+row*FISH_SHOP_LIST_ROW_H;
        bool selected=id==item;
        if(selected) {
            box(tile,FISH_SHOP_LIST_X+2,y,FISH_SHOP_LIST_W-4,FISH_SHOP_LIST_ROW_H,RGB(71,61,42));
            box(tile,FISH_SHOP_LIST_X,y+2,2,FISH_SHOP_LIST_ROW_H-4,SUN);
        }
        caption(tile,68,y,fish_shop_name(id),selected?SUN:RPG_LIGHT,selected?RGB(71,61,42):RPG_DARK);
    }
    box(tile,0,28,240,21,RPG_DARK);caption(tile,120,29,fish_merchant_says(g),RPG_LIGHT,RPG_DARK);
    box(tile,0,163,240,57,RPG_DARK);box(tile,7,163,226,1,RPG_BRASS);
    box(tile,8,170,48,46,RGB(35,30,25));
    if(fish_shop_list_goods[item]) {
        arm_2d_point_float_t icon_centre={32,193};
        arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&self->shop_detail_operation,tile,NULL,&icon_centre);
        ARM_2D_OP_WAIT_ASYNC(&self->shop_detail_operation);
    } else {
        line(tile,44,193,20,193,RPG_LIGHT);line(tile,20,193,30,183,RPG_LIGHT);line(tile,20,193,30,203,RPG_LIGHT);
    }
    const char *item_name=item==FISH_SHOP_CARGO?fish_containers[g->economy.container].name:fish_shop_name(item);
    arm_2d_size_t name_size=arm_lcd_get_string_line_box(item_name,fish_remarks_font());
    chinese_text_tinted(tile,60+name_size.iWidth/2,164,item_name,name_size,RPG_LIGHT,RPG_DARK);
    if(item==0) snprintf(label,sizeof(label),"+%lu金币",(unsigned long)fish_hold_value(&g->economy));
    else if(kind==FISH_SHOP_BAIT) snprintf(label,sizeof(label),"%lu金币/5份",(unsigned long)fish_shop_price(item));
    else if(kind==FISH_SHOP_DRINK) snprintf(label,sizeof(label),"%lu金币",(unsigned long)fish_shop_price(item));
    else if(item==3 && fish_shop_free_line(g))snprintf(label,sizeof(label),"免费补给");
    else if(kind<FISH_SHOP_BAIT) snprintf(label,sizeof(label),"%lu金币%s",(unsigned long)fish_shop_price(item),
        fish_equipment_owned(&g->economy,kind,index)?" 已拥有":"");
    else if(kind==FISH_SHOP_CONTAINER) snprintf(label,sizeof(label),"%lu金币%s",(unsigned long)fish_shop_price(item),g->economy.container>=index?" 已拥有":"");
    else if(item==FISH_SHOP_CARGO) {
        char weight[20];weight_text(weight,sizeof(weight),fish_hold_weight(&g->economy));
        snprintf(label,sizeof(label),"%s/%lu kg",weight,(unsigned long)fish_hold_capacity(&g->economy)/1000);
    }
    else snprintf(label,sizeof(label),"返回");
    arm_2d_size_t price_size=arm_lcd_get_string_line_box(label,fish_remarks_font());
    ui_text(tile,234-price_size.iWidth,164,174-name_size.iWidth-4,SUN,RPG_DARK,label);
    char detail[96];const char *description=detail;
    if(kind==FISH_SHOP_ROD) description=fish_rod_descriptions[index];
    else if(kind==FISH_SHOP_FLOAT) description=fish_float_descriptions[index];
    else if(kind==FISH_SHOP_BAIT) description=fish_baits[index].description;
    else if(kind==FISH_SHOP_LINE && index==FISH_PE_LINE)description="像一条钢缆，会吓跑小鱼";
    else if(kind==FISH_SHOP_LINE) snprintf(detail,sizeof(detail),"线径0.%02u毫米 隐蔽%s",fish_line_sets[index].diameter_hundredths_mm,
        fish_line_sets[index].bite_percent[0]>=90?"高":"低");
    else if(kind==FISH_SHOP_CONTAINER) description="扩容并保留原有鱼获";
    else if(kind==FISH_SHOP_DRINK) description="收放线和提竿窗口+10%";
    else description=item==0?"出售容器内和船尾鱼获":item==FISH_SHOP_CARGO?"查看容器内和船尾鱼获":"回到岸边继续垂钓";
    caption(tile,148,181,description,RPG_LIGHT,RPG_DARK);
    if(kind==FISH_SHOP_ROD) {
        snprintf(detail,sizeof(detail),"抛距%u像素 收线%u%%",fish_rods[index].max_cast_px,fish_rods[index].reel_percent);
        caption(tile,148,200,detail,SUN,RPG_DARK);
    } else {
        const char *effect=detail;
        if(kind==FISH_SHOP_FLOAT) {
            if(index==1) snprintf(detail,sizeof(detail),"提竿窗口 +%u%%",fish_floats[index].strike_window_bonus_percent);
            else snprintf(detail,sizeof(detail),"夜间咬钩 +%u%%",fish_floats[index].night_bite_bonus_percent);
        }
        else if(kind==FISH_SHOP_LINE) {
            unsigned cm=fish_line_sets[index].distance_limit_cm;
            snprintf(detail,sizeof(detail),"线长%u.%u米 强%u 窗%u%%",cm/100,cm%100/10,fish_line_sets[index].strength,fish_line_sets[index].strike_window_percent);
        }
        else if(kind==FISH_SHOP_DRINK) {
            unsigned minutes=(g->latte_remaining_ms+999)/1000;
            if(minutes)snprintf(detail,sizeof(detail),"剩余%u:%02u 再饮刷新6时",minutes/60,minutes%60);
            else snprintf(detail,sizeof(detail),"持续6小时 买后即饮");
        }
        else if(kind==FISH_SHOP_BAIT) snprintf(detail,sizeof(detail),"库存%u份 每包5份",g->economy.bait_stock[index]);
        else if(kind==FISH_SHOP_CONTAINER) snprintf(detail,sizeof(detail),"容量%lu kg",(unsigned long)fish_containers[index].capacity_g/1000);
        else if(item==FISH_SHOP_BACK) effect="短按返回岸边";
        else {
            char weight[20];weight_text(weight,sizeof(weight),fish_hold_weight(&g->economy));
            snprintf(detail,sizeof(detail),"总重%s",weight);
        }
        if(kind==FISH_SHOP_LINE) ui_text(tile,60,200,176,SUN,RPG_DARK,effect);
        else caption(tile,148,200,effect,SUN,RPG_DARK);
    }
}
static void hold_panel(const arm_2d_tile_t *tile,const fish_scene_t *self)
{
    const fish_game_t *g=&self->game;if(g->mode!=FISH_HOLD) return;
    const fish_economy_t *e=&g->economy;
    unsigned pages=fish_hold_page_count(e),i=fish_hold_page_index(e,g->guide_index);
    unsigned species=i<FISH_SPECIES_COUNT?i:e->hanging_species;
    uint16_t border=species<FISH_SPECIES_COUNT?fish_rarity_colors[fish_species_profile(i<FISH_SPECIES_COUNT?g->map_index:e->hanging_map,species)->rarity]:RPG_BRASS;
    /* Use the collection card's frame and quiet ink background. The storage
     * item is a small header icon, never a texture behind the fish. */
    box(tile,27,43,186,157,SHADOW);box(tile,25,41,186,157,border);box(tile,27,43,182,153,INK);
    if(species<FISH_SPECIES_COUNT && fish_species_is_variant(species))special_frame(tile,25,41,186,157);
    arm_2d_point_float_t centre={29,20};
    arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&self->container_operation,tile,NULL,&centre);
    ARM_2D_OP_WAIT_ASYNC(&self->container_operation);
    caption(tile,96,3,fish_containers[e->container].name,RPG_LIGHT,RPG_DARK);
    coin_count(tile,153,9,e->coins);
    char label[48],weight[20];weight_text(weight,sizeof(weight),fish_hold_weight(e));
    snprintf(label,sizeof(label),"%s / %lu.0 kg",weight,(unsigned long)fish_hold_capacity(e)/1000);
    caption(tile,137,23,label,SUN,RPG_DARK);
    if(species<FISH_SPECIES_COUNT && (i==FISH_SPECIES_COUNT || e->count[i])) {
        arm_2d_point_float_t portrait_centre={120,80};
        arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&self->hold_portrait_operation,tile,NULL,&portrait_centre);
        ARM_2D_OP_WAIT_ASYNC(&self->hold_portrait_operation);
        caption(tile,120,110,fish_species[species].name_zh,CREAM,INK);
        uint32_t grams=i<FISH_SPECIES_COUNT?e->grams[i]:e->hanging_grams;
        uint32_t value=i<FISH_SPECIES_COUNT?e->value[i]:e->hanging_value;
        weight_text(weight,sizeof(weight),grams);
        snprintf(label,sizeof(label),"%u条 共%s",i<FISH_SPECIES_COUNT?e->count[i]:1,weight);
        caption(tile,120,130,label,CREAM,INK);
        snprintf(label,sizeof(label),"价值%lu金币",(unsigned long)value);caption(tile,120,147,label,SUN,INK);
    } else caption(tile,120,87,"鱼库空空，去钓一条吧",RPG_MUTED,INK);
    if(pages) caption(tile,120,164,i==FISH_SPECIES_COUNT?"船尾大鱼：限一条":"装满后自动放流",RPG_LIGHT,INK);
    snprintf(label,sizeof(label),"%02u/%02u 总价%lu金币",pages?g->guide_index+1:0,pages,(unsigned long)fish_hold_value(e));
    ui_text(tile,34,180,174,SUN,INK,label);
}
static void money_feedback(const arm_2d_tile_t *tile,const fish_game_t *g)
{
    if(g->mode!=FISH_SHOP) return;
    for(unsigned i=0;i<FISH_MONEY_POPUPS;i++) {
        const fish_money_popup_t *p=&g->money.popup[i];
        uint32_t age=g->now-p->at;
        if(!p->amount || age>=FISH_MONEY_LIFETIME_MS) continue;
        char label[24];snprintf(label,sizeof(label),"%+ldG",(long)p->amount);
        arm_2d_size_t size=arm_lcd_get_string_line_box(label,fish_remarks_font());
        int x=184-size.iWidth/2,y=143-(int)(age*48/FISH_MONEY_LIFETIME_MS);
        uint8_t opacity=age<650?255:(uint8_t)((FISH_MONEY_LIFETIME_MS-age)*255/250);
        arm_lcd_text_set_opacity(opacity);
        ui_text(tile,x+1,y+1,104,RPG_DARK,RPG_DARK,label);
        ui_text(tile,x,y,104,p->amount>0?RGB(169,235,142):RGB(255,192,130),RPG_DARK,label);
        if(p->aggregate) ui_text(tile,154,y-14,76,RPG_LIGHT,RPG_DARK,"旧鱼获合计");
        arm_lcd_text_set_opacity(255);
    }
}
static void hanging_fish(const arm_2d_tile_t *tile,const fish_scene_t *self)
{
    const fish_game_t *g=&self->game;if(fish_game_ashore(g) || g->economy.hanging_species>=FISH_SPECIES_COUNT) return;
    int x=g->boat_x-fish_game_camera(g)-g->facing*25;
    int bob=(g->now/450)%4;bob=bob==3?1:bob;
    arm_2d_point_float_t centre={x-g->facing*4,water_y(g)+12+bob};
    arm_2dp_tile_transform_xy((arm_2d_op_trans_t *)&self->hanging_operation,tile,NULL,&centre);
    ARM_2D_OP_WAIT_ASYNC(&self->hanging_operation);
    /* Two visible rope strands from the stern cleat to the head/gill tie. */
    line(tile,x,water_y(g)-2+bob,(int)centre.fX-2,(int)centre.fY-3,WOOD_LIGHT);
    line(tile,x+1,water_y(g)-2+bob,(int)centre.fX+2,(int)centre.fY+2,RPG_BRASS);
    line(tile,(int)centre.fX-3,(int)centre.fY-2,(int)centre.fX+3,(int)centre.fY+2,WOOD_LIGHT);
    box(tile,x-2,water_y(g)-3+bob,4,2,WOOD);
}
static void catch_card(const arm_2d_tile_t *tile, fish_scene_t *self, bool mirror)
{
    const fish_game_t *game=&self->game;
    bool guide=game->mode==FISH_GUIDE;
    if (game->mode!=FISH_CAUGHT && !guide) return;
    unsigned index=guide?game->guide_index:game->last_species;
    bool known=!guide || game->species_catches[index]>0;
    const fish_species_t *species=fish_species_profile(guide?game->species_best_map[index]:game->last_map,index);
    const fish_species_art_t *art=fish_species_portrait_art(index,self->portrait_scale_q8);
    uint16_t frame_color=fish_rarity_colors[species->rarity];
    bool special=known && fish_species_is_variant(index);
    box(tile,27,43,186,157,SHADOW); box(tile,25,41,186,157,frame_color);
    if(special) special_frame(tile,25,41,186,157);
    box(tile,27,43,182,153,INK);
    char label[48];
    static const char *tiers[]={"铜","银","金"};
    snprintf(label,sizeof(label),"%s%s",special?"稀有十倍":tiers[species->rarity],guide?"":" 收获");
    if (known && !(guide && game->guide_page)) {
        int scale=self->portrait_scale_q8,height=species->portrait_height_px*scale/256;
        int x=120+(mirror?-1:1)*(48-species->portrait_centre_x)*scale/256;
        int y=95+(28-species->portrait_centre_y)*scale/256;
        if (height>56) y-=(height-56)/2; /* Giants grow upward, leaving captions clear. */
        scaled_species_bitmap(tile,art->sprite,art->mask,x,y,mirror,scale*96/art->sprite->tRegion.tSize.iWidth,&self->portrait_operation);
    }
    else if(!game->guide_page || !guide) {
        static const uint8_t question[7]={14,17,1,2,4,0,4};
        for (unsigned y=0;y<7;y++) for (unsigned x=0;x<5;x++)
            if (question[y]&(1u<<(4-x))) box(tile,112+x*3,84+y*3,3,3,HILL);
    }
    if(guide && game->guide_page) {
        caption(tile,120,48,known?species->name_zh:"未收集",CREAM,INK);
        if(known) {
            for(unsigned i=0;i<4;i++) {
                const char *field=fish_guide_fields[index][i];
                if(game->map_index && i==1) {
                    if(index==0)field="水域：河岸草湾";
                    else if(index==1 || index==13)field="水域：江心深槽";
                    else if(index==7)field="水域：桥下开阔水面";
                }
                ui_text(tile,34,83+(int)i*27,174,i==3?SUN:CREAM,INK,field);
            }
        } else {
            char hint[64];snprintf(hint,sizeof(hint),"线索：%s",fish_guide_hints[index]);
            caption(tile,120,91,hint,SUN,INK);
            caption(tile,120,133,"钓获后解锁习性",HILL,INK);
        }
        goto guide_record;
    }
    arm_2d_size_t title_size=arm_lcd_get_string_line_box(label,fish_remarks_font());
    ui_text(tile,120-title_size.iWidth/2,48,title_size.iWidth,frame_color,INK,label);
    caption(tile,120,125,known?species->name_zh:"未收集",CREAM,INK);
    const char *metric=species->size_metric==FISH_SIZE_TOTAL_LENGTH?"全长":
        species->size_metric==FISH_SIZE_CARAPACE_LENGTH?"背甲长":"头胸甲宽";
    if (known) {
        char weight[20];weight_text(weight,sizeof(weight),self->card_weight_g);
        snprintf(label,sizeof(label),"%s%u.%u厘米 %s",metric,self->card_size_mm/10,self->card_size_mm%10,weight);
    }
    else snprintf(label,sizeof(label),"暂无记录");
    arm_2d_size_t metric_size=arm_lcd_get_string_line_box(label,fish_remarks_font());
    ui_text(tile,120-metric_size.iWidth/2,144,178,SUN,INK,label);
    uint8_t stars=known?fish_species_stars(species,self->card_size_mm):0;
    static const uint16_t star_rows[9]={0x010,0x038,0x038,0x1ff,0x0fe,0x07c,0x07c,0x0c6,0x082};
    for (unsigned star=0;star<5;star++) {
        uint16_t color=star<stars?SUN:HILL;
        for (unsigned row=0;row<9;row++) {
            for (unsigned col=0;col<9;) {
                if (!(star_rows[row]&(1u<<col))) { ++col; continue; }
                unsigned start=col;
                while (col<9 && (star_rows[row]&(1u<<col))) ++col;
                box(tile,90+star*13+start,163+row,col-start,1,color);
            }
        }
    }
    if (known) chinese_text(tile,120,176,fish_species_remarks[index],self->remarks_box);
    if(!guide) {
        const char *cargo=game->cargo_result==FISH_HANGING?"大鱼挂在船尾":game->cargo_result==FISH_RELEASED?"装不下，已放流":"已收入容器";
        caption(tile,120,199,cargo,game->cargo_result==FISH_RELEASED?RED:SUN,INK);
    }
guide_record:
    if (guide) {
        snprintf(label,sizeof(label),"钓获%u次",game->species_catches[index]);
        ui_text(tile,8,24,90,HILL,INK,label);
    }
    if(known) {
        box(tile,100,28,134,13,INK);
        ui_text(tile,104,24,34,SUN,INK,guide?"纪录":"售价");
        coin_count(tile,139,30,fish_catch_price_on(guide?game->species_best_map[index]:game->last_map,index,self->card_size_mm));
    }
}

static void smoke_strike(fish_game_t *game,uint32_t at)
{
    game->phase_at=at-fish_species[game->pending_species].strike_delay_ms;
    fish_game_input(game,FISH_PRESS,0,at);
}


extern const arm_2d_tile_t c_tileRouteWetlandRGB565,c_tileRouteCityRGB565;
static void resource_update(fish_scene_t *self)
{
    fish_game_t *g=&self->game;
    if(!self->media_started) {
        fish_media_begin(g->now);self->media_started=true;self->resource_boot=true;
        g->previous_mode=fish_game_ashore(g)?FISH_SHORE:FISH_MOVE;
        g->travel_target=g->map_index;g->travel_paid=false;g->travel_requested=true;
        g->mode=FISH_LOADING;g->pressed=false;
    }
    fish_media_tick(g->now,self->save.writing);
    if(!fish_media_ready())return;
    if(!self->media_complete) {
        self->media_complete=true;
        /* A previously unavailable SD can now contain the user's old saves. */
        if(self->resource_boot && self->save.blocked && self->save.status==FISH_SAVE_ERROR) {
            fish_save_init(&self->save,g,g->now);g->travel_target=g->map_index;
            g->previous_mode=fish_game_ashore(g)?FISH_SHORE:FISH_MOVE;g->mode=FISH_LOADING;
        }
        /* Present the destination page before the first NOR configuration. */
        return;
    }
    if(!fish_resources_valid() && g->mode!=FISH_ROUTE && g->mode!=FISH_LOADING) {
        self->resource_boot=true;g->travel_target=g->map_index;g->travel_paid=false;
        g->previous_mode=fish_game_ashore(g)?FISH_SHORE:FISH_MOVE;
        g->travel_requested=true;g->pressed=false;g->mode=FISH_LOADING;
    }
    if(g->travel_requested && !self->resource_loading) {
        fish_resources_begin(g->travel_target);self->resource_loading=true;
    }
    if(!self->resource_loading)return;
    fish_resources_tick();
    if(fish_resources_phase()==FISH_RES_DONE || fish_resources_phase()==FISH_RES_ERROR) {
        bool ok=fish_resources_phase()==FISH_RES_DONE;
        if(!ok && fish_resources_source_error()) {
            self->resource_loading=false;fish_media_require(g->now);return;
        }
        if(ok && self->resource_boot) {
            g->mode=g->previous_mode==FISH_SHORE?FISH_SHORE:FISH_MOVE;g->travel_requested=false;
            g->pressed=false;g->clock_at=g->simulation_at=g->now;
        } else fish_game_travel_complete(g,ok);
        self->resource_loading=false;
        if(ok)self->resource_boot=false;
    }
}
static void opacity_box(const arm_2d_tile_t *tile,int x,int y,int w,int h,uint16_t color,unsigned alpha)
{
    arm_2d_region_t region={.tLocation={x,y},.tSize={w,h}};
    arm_2d_rgb565_fill_colour_with_opacity(tile,&region,(arm_2d_color_rgb565_t){.tValue=color},alpha);
    ARM_2D_OP_WAIT_ASYNC();
}
static void route_number(const arm_2d_tile_t *tile,int x,int y,unsigned number,uint16_t ink,uint16_t paper)
{
    char value[8];snprintf(value,sizeof(value),"%02u",number);
    arm_2d_region_t region={.tLocation={x,y},.tSize={18,10}};
    arm_lcd_text_set_target_framebuffer((arm_2d_tile_t *)tile);
    arm_lcd_text_set_font(&ARM_2D_FONT_6x8.use_as__arm_2d_font_t);
    arm_lcd_text_set_draw_region(&region);arm_lcd_text_set_colour(ink,paper);
    arm_lcd_text_location(0,0);arm_lcd_puts(value);ARM_2D_OP_WAIT_ASYNC();
}
static void route_offscreen(const arm_2d_tile_t *tile,int x,int y,unsigned index,uint16_t ink,uint16_t paper)
{
    enum {W=FISH_ROUTE_VIEW_WIDTH,H=FISH_ROUTE_VIEW_HEIGHT};
    int dx=x-W/2,dy=y-H/2,ax=abs(dx),ay=abs(dy);
    int scale_x=ax?(W/2-10)*1024/ax:32767,scale_y=ay?(H/2-10)*1024/ay:32767;
    int scale=scale_x<scale_y?scale_x:scale_y;
    int px=W/2+dx*scale/1024,py=H/2+dy*scale/1024;
    if(ay>ax) {
        int direction=dy>0?1:-1;
        line(tile,px-3,py-direction*3,px,py,ink);line(tile,px,py,px+3,py-direction*3,ink);
        route_number(tile,px-6,py+(direction>0?-14:7),index+1,ink,paper);
    } else {
        int direction=dx>0?1:-1;
        line(tile,px-direction*3,py-3,px,py,ink);line(tile,px,py,px-direction*3,py+3,ink);
        route_number(tile,px+(direction>0?-21:7),py-3,index+1,ink,paper);
    }
}
static void route_geography(const arm_2d_tile_t *tile,const fish_route_view_t *view)
{
    const uint16_t bank=RGB(126,158,164),water=RGB(177,199,194);
    const uint16_t hill=RGB(166,153,113),trees=RGB(143,160,118),town=RGB(175,174,148);
    /* Cartographic ink only: no extra destinations or physical map geometry.
     * All symbols share the paper's world coordinates and viewport clipping. */
    static const fish_route_stop_t river[]={
        {328,108},{328,136},{336,152},{340,170},{334,188},
        {338,208},{336,230},{322,244},{312,264}
    };
    for(unsigned i=1;i<sizeof(river)/sizeof(river[0]);i++) {
        int y0=river[i-1].y-view->y,y1=river[i].y-view->y;
        int dx=river[i].x-river[i-1].x,dy=river[i].y-river[i-1].y;
        int first=y0<0?0:y0,last=y1>FISH_ROUTE_VIEW_HEIGHT?FISH_ROUTE_VIEW_HEIGHT:y1;
        for(int y=first;y<last;y++) {
            int x=river[i-1].x-view->x+dx*(y-y0)/dy;
            box(tile,x-4,y,9,1,bank);box(tile,x-3,y,7,1,water);
        }
    }
    static const struct {int16_t x,y;uint8_t height;} mountains[]={
        {233,127,11},{245,131,8},{370,131,13},{388,127,16},{406,133,10}
    };
    for(unsigned i=0;i<sizeof(mountains)/sizeof(mountains[0]);i++) {
        int x=mountains[i].x-view->x,y=mountains[i].y-view->y,h=mountains[i].height;
        line(tile,x-h/2,y+h,x,y,hill);line(tile,x,y,x+h/2,y+h,hill);
        line(tile,x-h/2,y+h,x+h/2,y+h,hill);
        line(tile,x,y+4,x+2,y+7,hill);line(tile,x+2,y+7,x+3,y+5,hill);
    }
    static const fish_route_stop_t forest[]={
        {234,161},{241,173},{352,140},{359,153},{236,234},{245,239},{253,232}
    };
    for(unsigned i=0;i<sizeof(forest)/sizeof(forest[0]);i++) {
        int x=forest[i].x-view->x,y=forest[i].y-view->y;
        line(tile,x-3,y+4,x,y,trees);line(tile,x,y,x+3,y+4,trees);
        line(tile,x-4,y+7,x,y+2,trees);line(tile,x,y+2,x+4,y+7,trees);
        box(tile,x-3,y+7,7,1,trees);box(tile,x,y+7,1,4,trees);
    }
    /* Small wetland pond, water strokes and reed clumps around the lakeside. */
    int px=240-view->x,py=217-view->y;
    box(tile,px-10,py-2,21,5,water);box(tile,px-7,py-4,15,9,water);
    box(tile,px-7,py-5,15,1,bank);box(tile,px-7,py+5,15,1,bank);
    box(tile,px-11,py-1,1,3,bank);box(tile,px+11,py-1,1,3,bank);
    box(tile,px-4,py-1,5,1,bank);box(tile,px+3,py+2,4,1,bank);
    static const fish_route_stop_t reeds[]={{234,191},{264,207},{263,220},{419,163}};
    for(unsigned i=0;i<sizeof(reeds)/sizeof(reeds[0]);i++) {
        int x=reeds[i].x-view->x,y=reeds[i].y-view->y;
        box(tile,x,y,1,8,trees);box(tile,x+4,y+2,1,6,trees);
        line(tile,x-3,y+2,x,y+6,trees);line(tile,x+7,y+3,x+4,y+6,trees);
        box(tile,x,y-2,1,3,hill);box(tile,x+4,y,1,3,hill);
    }
    /* A few urban blocks and a printed north arrow balance the eastern side. */
    int tx=429-view->x,ty=192-view->y;
    for(int row=0;row<3;row++)for(int col=0;col<2;col++) {
        int x=tx+col*7,y=ty+row*7;
        box(tile,x,y,5,1,town);box(tile,x,y+4,5,1,town);
        box(tile,x,y,1,5,town);box(tile,x+4,y,1,5,town);
    }
    int nx=434-view->x,ny=127-view->y;
    line(tile,nx,ny,nx,ny+13,hill);line(tile,nx-3,ny+5,nx,ny,hill);
    line(tile,nx,ny,nx+3,ny+5,hill);
}

static void route_sheet(const arm_2d_tile_t *tile,const fish_game_t *g,const fish_route_view_t *view)
{
    const uint16_t paper=RPG_PAPER,ink=RPG_INK,accent=RGB(137,65,45);
    const uint16_t grid=RGB(213,201,169),light=RGB(230,218,187),rail=RGB(69,112,101);
    enum {W=FISH_ROUTE_VIEW_WIDTH,H=FISH_ROUTE_VIEW_HEIGHT};
    box(tile,0,0,W,H,paper);
    /* Printed reference grid and folds are anchored to the 640x480 sheet. */
    for(int x=0;x<=FISH_ROUTE_WIDTH;x+=64)box(tile,x-view->x,0,1,H,grid);
    for(int y=0;y<=FISH_ROUTE_HEIGHT;y+=64)box(tile,0,y-view->y,W,1,grid);
    for(int x=160;x<FISH_ROUTE_WIDTH;x+=160) {
        box(tile,x-view->x,0,1,H,RPG_PAPER_SHADE);box(tile,x-view->x+1,0,1,H,light);
    }
    for(int y=120;y<FISH_ROUTE_HEIGHT;y+=120) {
        box(tile,0,y-view->y,W,1,RPG_PAPER_SHADE);box(tile,0,y-view->y+1,W,1,light);
    }
    for(unsigned i=0;i<70;i++) {
        int x=17+(i*97)%604-view->x,y=20+(i*71)%440-view->y;
        if(x>=0 && x<W && y>=0 && y<H)box(tile,x,y,2,1,i&1?light:grid);
    }
    route_geography(tile,view);
    for(unsigned i=0;i<FISH_MAP_COUNT;i++) {
        int x=fish_route_stops[i].x-view->x,y=fish_route_stops[i].y-view->y;
        bool selected=i==g->route_index;
        box(tile,x-33,y-35,66,34,selected?accent:RPG_MUTED);
        box(tile,x-32,y-34,64,32,paper);
        arm_2d_region_t at={.tLocation={x-32,y-34},.tSize={64,32}};
        arm_2d_rgb565_tile_copy_with_opacity(i?&c_tileRouteCityRGB565:&c_tileRouteWetlandRGB565,tile,&at,
            selected?255:192);ARM_2D_OP_WAIT_ASYNC();
    }
    /* Diagrammatic railway: straight segments and one 45-degree turn, not
     * physical fishing distance. Drawing stays within the visible child tile. */
    static const fish_route_stop_t path[]={{280,180},{280,196},{314,230},{388,230}};
    for(unsigned i=1;i<sizeof(path)/sizeof(path[0]);i++) {
        int x=path[i-1].x-view->x,y=path[i-1].y-view->y;
        int dx=path[i].x-path[i-1].x,dy=path[i].y-path[i-1].y;
        if(!dx)box(tile,x-1,y,3,dy+1,rail);
        else if(!dy)box(tile,x,y-1,dx+1,3,rail);
        else for(int d=0;d<=dx;d++)if(x+d>=-1 && x+d<W && y+d>=-1 && y+d<H)
            box(tile,x+d-1,y+d-1,3,3,rail);
    }
    /* Railings mark the river crossing, keeping the route stroke dominant. */
    box(tile,330-view->x,227-view->y,13,1,RPG_MUTED);
    box(tile,330-view->x,233-view->y,13,1,RPG_MUTED);
    box(tile,330-view->x,227-view->y,1,7,RPG_MUTED);
    box(tile,342-view->x,227-view->y,1,7,RPG_MUTED);
    for(unsigned i=0;i<FISH_MAP_COUNT;i++) {
        int x=fish_route_stops[i].x-view->x,y=fish_route_stops[i].y-view->y;
        bool selected=i==g->route_index;uint16_t color=selected?accent:ink;
        if(x<8 || x>W-8 || y<8 || y>H-8) {
            route_offscreen(tile,x,y,i,selected?accent:rail,paper);continue;
        }
        int r=selected?5:3;
        box(tile,x-r+1,y-r,2*r-1,2*r+1,color);box(tile,x-r,y-r+1,2*r+1,2*r-1,color);
        box(tile,x-r+2,y-r+2,2*r-3,2*r-3,paper);
        if(i==g->map_index)box(tile,x-1,y-1,3,3,accent);
        caption(tile,x,y-57,fish_map_names[i],color,paper);
        if(selected)box(tile,x-24,y-38,48,1,accent);
        route_number(tile,x-30,y+5,i+1,color,paper);
        if(i==g->map_index)caption(tile,x+24,y+3,"当前",accent,paper);
    }
}
static void route_page(const arm_2d_tile_t *tile,const fish_game_t *g,const fish_route_view_t *view)
{
    const uint16_t paper=RPG_PAPER,accent=RGB(137,65,45),sign=RGB(51,77,69);
    box(tile,0,0,240,240,RPG_DARK);box(tile,5,4,230,232,paper);
    /* Station route-board masthead stays fixed while its printed map scrolls. */
    box(tile,5,4,230,31,sign);box(tile,5,35,230,2,RPG_BRASS);
    box(tile,15,12,12,12,CREAM);box(tile,17,14,8,4,sign);box(tile,18,21,2,2,sign);box(tile,23,21,2,2,sign);
    line(tile,18,24,16,27,CREAM);line(tile,24,24,26,27,CREAM);
    ui_text(tile,35,8,120,CREAM,sign,"车站路线");
    box(tile,158,9,69,19,RPG_DARK);coin_count(tile,164,14,g->economy.coins);
    arm_2d_region_t region={.tLocation={FISH_ROUTE_VIEW_X,FISH_ROUTE_VIEW_Y},
        .tSize={FISH_ROUTE_VIEW_WIDTH,FISH_ROUTE_VIEW_HEIGHT}};
    arm_2d_tile_t window;
    arm_2d_tile_generate_child(tile,&region,&window,false);
    route_sheet(&window,g,view);
    /* Viewport rails show where this window lies on the larger sheet. */
    box(tile,8,176,224,1,RPG_PAPER_SHADE);box(tile,232,40,1,136,RPG_PAPER_SHADE);
    int overview_x,overview_y;
    if(!fish_route_overview(&overview_x,&overview_y)) {
        box(tile,8+view->x*224/FISH_ROUTE_WIDTH,176,224*224/FISH_ROUTE_WIDTH,2,RPG_MUTED);
        box(tile,232,40+view->y*136/FISH_ROUTE_HEIGHT,2,136*136/FISH_ROUTE_HEIGHT,RPG_MUTED);
    }
    caption(tile,120,179,fish_map_names[g->route_index],accent,paper);
    const char *status=g->travel_error==1?"金币不足，需要180G":g->travel_error==2?"资源加载失败，短按重试":g->route_index==g->map_index?"当前钓点，短按返回":"每程180G  路程5小时";
    caption(tile,120,198,status,g->travel_error?accent:RPG_INK,paper);
    caption(tile,120,217,"旋转选择 短按乘车 长按返回",RPG_MUTED,paper);
}
static void game_icon(const arm_2d_tile_t *tile,int left,int top)
{
    /* Resident palette/indices also work with an empty or erased SD cache.
     * Integer 2x pixels preserve the game artwork's grid without a framebuffer. */
    unsigned at=0;
    for(unsigned y=0;y<FISH_GAME_ICON_HEIGHT;++y)for(unsigned x=0;x<FISH_GAME_ICON_WIDTH;) {
        unsigned run=c_bmpGameIconIndices[at++],color=run&31,width=(run>>5)+1;
        if(color)box(tile,left+x*2,top+y*2,width*2,2,c_bmpGameIconPalette[color]);
        x+=width;
    }
}
static void media_page(const arm_2d_tile_t *tile,fish_media_status_t status)
{
        fish_media_phase_t state=status.phase;
        box(tile,0,0,240,240,INK);
        bool checking=state==FISH_MEDIA_CHECK || state==FISH_MEDIA_VERIFY || state==FISH_MEDIA_SETTLE;
        caption(tile,120,25,state==FISH_MEDIA_FAULT?"上次为看门狗重启":checking?"正在检查资源包":status.receiving?"正在接收资源":"需要游戏资源",CREAM,INK);
        caption(tile,120,52,"版本 " FISH_ASSET_VERSION " D12",SUN,INK);
        char label[64];
        if(state==FISH_MEDIA_FAULT) {
            snprintf(label,sizeof(label),"S%02u  %08lx",status.reset_stage,(unsigned long)status.fault.detail);
            caption(tile,120,91,label,CREAM,INK);
            if(status.fault.cause)snprintf(label,sizeof(label),"PC %08lx",(unsigned long)status.fault.pc);
            else snprintf(label,sizeof(label),"PC --------");
            caption(tile,120,116,label,CREAM,INK);
            snprintf(label,sizeof(label),"C %lu  V %08lx",(unsigned long)(status.fault.cause?status.fault.cause-1:0),(unsigned long)status.fault.value);
            if(!status.fault.cause)snprintf(label,sizeof(label),"C -  V --------");
            caption(tile,120,141,label,CREAM,INK);
            caption(tile,120,190,"短按重试",SUN,INK);return;
        }
        if(status.watchdog_reset) {
            if(status.reset_stage) {snprintf(label,sizeof(label),"上次看门狗重启 S%02u",status.reset_stage);caption(tile,120,73,label,SUN,INK);}
            else caption(tile,120,73,"上次为看门狗重启",SUN,INK);
        }
        game_icon(tile,72,79);
        const char *message=state==FISH_MEDIA_ERROR?"SD读取失败，请重新上电":
            state==FISH_MEDIA_VERIFY || state==FISH_MEDIA_CHECK?(status.map?"校验城市资源 2/2":"校验湿地资源 1/2"):
            state==FISH_MEDIA_SETTLE?"资源已就绪，即将进入":
            status.error==FISH_RES_ERR_MISSING?(status.map?"缺少城市资源包":"缺少湿地资源包"):
            status.error==FISH_RES_ERR_VERSION?"资源版本不符":
            status.error==FISH_RES_ERR_SIZE?"资源大小不符":
            status.error==FISH_RES_ERR_PACK?"资源包不匹配":
            status.error==FISH_RES_ERR_CRC?"资源校验未通过":
            status.error==FISH_RES_ERR_IO?"SD读取失败，请重试":"请拖入资源包";
        if(status.receiving) {snprintf(label,sizeof(label),"已接收 %lu KB",(unsigned long)(status.received_bytes/1024));message=label;}
        caption(tile,120,169,message,CREAM,INK);
        if(checking || status.receiving) {
            box(tile,29,195,182,8,SHADOW);
            if(status.receiving)box(tile,29+(status.elapsed_ms/80)%135,195,48,8,GREEN);
            else box(tile,29,195,status.progress*182/100,8,GREEN);
            if(status.receiving)caption(tile,120,218,"复制结束后自动校验",CREAM,INK);
            else {snprintf(label,sizeof(label),"%u%%  已用%lus",status.progress,(unsigned long)(status.elapsed_ms/1000));caption(tile,120,218,label,CREAM,INK);}
        } else if(state!=FISH_MEDIA_ERROR) {
            caption(tile,120,195,"解压后复制整个资源目录",SUN,INK);
            caption(tile,120,218,status.error==FISH_RES_ERR_MISSING?
                (status.map?"maps/1_city_river.fgpk":"maps/0_wetland.fgpk"):
                status.error==FISH_RES_ERR_NONE?"完成后自动进入游戏":"请使用配套资源包",CREAM,INK);
        }
}
static void loading_page(const arm_2d_tile_t *tile,fish_scene_t *self)
{
    const fish_game_t *g=&self->game;box(tile,0,0,240,240,INK);
    if(!fish_media_ready() && !fish_smoke_test) {
        media_page(tile,fish_media_status());return;
    }
    caption(tile,120,31,"正在前往",CREAM,INK);caption(tile,120,54,fish_map_names[g->travel_target],SUN,INK);
    unsigned n=self->resource_loading || fish_smoke_test?fish_resources_progress():0;
    /* Reuse the resident seated-fisherman icon while either map cache changes. */
    game_icon(tile,72,74);
    box(tile,27,156,186,12,HILL);box(tile,29,158,182,8,SHADOW);box(tile,29,158,n*182/100,8,GREEN);
    char value[16];snprintf(value,sizeof(value),"%u%%",n);caption(tile,120,174,value,SUN,INK);
    static const char *const descriptions[2][5]={
        {"正在整理钓具","正在前往码头","芦苇深处，或许藏着大鱼","风吹过湖面，鱼也慢了下来","熟悉的码头就在前面"},
        {"正在整理钓具","正在前往码头","宽广水域，或许藏着巨物","桥影下，也许有鱼正在等饵","灯火亮起，江面渐渐安静"}
    };
    unsigned tip=(g->now/7000)%5;
    if(g->travel_target && tip>=2) {
        unsigned arrival=(g->game_minute+(g->travel_paid?FISH_TRAVEL_MINUTES:0))%1440;
        tip=arrival>=1080 || arrival<360?4:2+(tip-2)%2;
    }
    caption(tile,120,198,descriptions[g->travel_target][tip],CREAM,INK);

}

static void frame_start(arm_2d_scene_t *s)
{
    fish_scene_t *self=(fish_scene_t *)s;
    fish_game_t *game=&self->game;
    fish_game_update(game,fish_hpm_millis());
    resource_update(self);
    fish_hpm_feedback_update(game);
    if(fish_media_ready() || self->save.writing)fish_save_tick(&self->save,game,game->now);
    self->save_box=arm_lcd_get_string_line_box(fish_save_label(&self->save),fish_remarks_font());
    fish_route_view_update(&self->route_view,game->mode==FISH_ROUTE,game->route_index,game->now);
    if(game->mode==FISH_ROUTE || game->mode==FISH_LOADING) return;
    self->hint_box=arm_lcd_get_string_line_box(control_hint(game),fish_remarks_font());
    arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->cloud_operation,
        &c_tileCloudsRGB565,&c_tileCloudsAlphaGRAY8,(arm_2d_point_float_t){46,6},0,2,2);
    if (fish_smoke_test && (self->hint_box.iWidth>222 || self->hint_box.iHeight>19)) {
        fprintf(stderr,"Control hint does not fit HUD: %d x %d\n",self->hint_box.iWidth,self->hint_box.iHeight); exit(1);
    }
    arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->footpath_operation,
        &c_tileFootpathRGB565,&c_tileFootpathAlphaGRAY8,(arm_2d_point_float_t){25,24},0,2,2);
    arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->exterior_operation,
        game->map_index && !fish_game_vendor_open(game,FISH_VENDOR_CONVENIENCE)?&c_tileCityShopClosedRGB565:&c_tileShopExteriorRGB565,&c_tileShopExteriorAlphaGRAY8,(arm_2d_point_float_t){33,25},0,2,2);
    arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->station_operation,
        &c_tileStationRGB565,&c_tileStationAlphaGRAY8,(arm_2d_point_float_t){55,27},0,2,2);
    arm_2dp_rgb565_tile_transform_xy_only_prepare(&self->exterior_closed_operation,
        &c_tileShopExteriorClosedPatchRGB565,(arm_2d_point_float_t){20,11.5f},0,2,2);
    if(game->map_index) {
        arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->park_lamp_operation,
            game->game_minute>=1080 || game->game_minute<360?&c_tileParkLampOnRGB565:&c_tileParkLampRGB565,
            &c_tileParkLampAlphaGRAY8,(arm_2d_point_float_t){5,42},0,2,2.35f);
        arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->park_light_operation,
            &c_tileParkLightRGB565,&c_tileParkLightAlphaGRAY8,(arm_2d_point_float_t){18,0},0,2,2);
    }
    for(unsigned side=0;side<2;side++)
        arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->dock_operation[side],
            &c_tileShoreDockRGB565,&c_tileShoreDockAlphaGRAY8,(arm_2d_point_float_t){29,25},0,side?-2:2,2);
    if (game->mode==FISH_BAG) {
        char attributes[48];
        self->guide_name_box=arm_lcd_get_string_line_box(bag_attributes(game,attributes,sizeof(attributes)),fish_remarks_font());
        if (fish_smoke_test && self->guide_name_box.iWidth>150) {fprintf(stderr,"Equipment attributes exceed detail strip\n");exit(1);}
        for (unsigned i=0;i<FISH_BAG_COUNT;i++) {
            char label[24]; const char *value=bag_value(game,i,label,sizeof(label));
            self->bag_names[i]=arm_lcd_get_string_line_box(value,fish_remarks_font());
            int max_width=i==FISH_BAG_ROD?82:i>=FISH_BAG_DOCK?(i==FISH_BAG_DOCK?76:38):136;
            if (fish_smoke_test && self->bag_names[i].iWidth>max_width) {
                fprintf(stderr,"Equipment name does not fit slot: item=%u width=%d value=%s\n",i,self->bag_names[i].iWidth,value); exit(1);
            }
        }
        arm_2dp_rgb565_tile_transform_xy_only_prepare(&self->backpack_operation,&c_tileBackpackOpenRGB565,
            (arm_2d_point_float_t){60.0f,44.0f},0.0f,2.0f,2.0f);
        const fish_ui_art_t *rod=&fish_equipment_art[game->rod_index];
        arm_2d_point_float_t pivot={12.0f,19.0f};
        arm_2d_err_t result=arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(
            &self->portrait_operation,rod->sprite,rod->mask,pivot,0.0f,2.0f,2.0f);
        if (fish_smoke_test && result<0) {fprintf(stderr,"Rod transform prepare failed: %d\n",result);exit(1);}
    }
    if(game->mode==FISH_SHOP) {
        if(fish_shop_list_goods[game->shop_index])
            arm_2dp_rgb565_tile_transform_xy_only_prepare(&self->shop_detail_operation,fish_shop_list_goods[game->shop_index],
                (arm_2d_point_float_t){8,8},0,2,2);
    }
    if(fish_game_ashore(game)) {
        const fish_ui_art_t *art=&fish_shore_player_art[fish_game_shore_pose(game)];
        arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->shore_player_operation,art->sprite,art->mask,
            (arm_2d_point_float_t){7,10},0,game->facing<0?-1.8f:1.8f,1.8f);
    }
    if(game->mode==FISH_HOLD) {
        const fish_ui_art_t *container=&fish_container_art[game->economy.container];
        arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->container_operation,container->sprite,container->mask,
            (arm_2d_point_float_t){16,18},0,1,1);
        unsigned item=fish_hold_page_index(&game->economy,game->guide_index);
        unsigned index=item<FISH_SPECIES_COUNT?item:game->economy.hanging_species;
        if(index<FISH_SPECIES_COUNT) {
            const uint8_t *bounds=fish_hold_portrait_bounds[index];
            unsigned scale=160u*256/bounds[2];
            unsigned height_scale=56u*256/bounds[3];
            if(scale>height_scale)scale=height_scale;
            const fish_species_art_t *art=fish_species_portrait_art(index,512);
            arm_2d_point_float_t pivot={bounds[0]/2.0f,bounds[1]/2.0f};
            float factor=scale/256.0f;
            arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->hold_portrait_operation,art->sprite,art->mask,
                pivot,0,factor,factor);
        }
    }
    if(game->economy.hanging_species<FISH_SPECIES_COUNT) {
        const fish_species_art_t *art=&fish_species_art[game->economy.hanging_species];
        arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(&self->hanging_operation,art->sprite,art->mask,
            (arm_2d_point_float_t){8,14},ARM_2D_ANGLE(game->facing>0?-30:30),-game->facing*1.1f,1.1f);
    }
    if (game->mode==FISH_CAUGHT || game->mode==FISH_GUIDE) {
        bool guide=game->mode==FISH_GUIDE;
        unsigned index=guide?game->guide_index:game->last_species;
        bool known=!guide || game->species_catches[index]>0;
        self->remarks_box=arm_lcd_get_string_line_box(fish_species_remarks[index],fish_remarks_font());
        if (guide) self->guide_name_box=arm_lcd_get_string_line_box(known?fish_species[index].name_zh:"未收集",fish_remarks_font());
        if (fish_smoke_test && (self->remarks_box.iWidth>178 || self->remarks_box.iHeight>19)) {
            fprintf(stderr,"Remark does not fit catch card\n"); exit(1);
        }
        if (!guide && !game->last_weight_g) game->last_weight_g=fish_species_weight_g(fish_species_profile(game->last_map,index),game->last_size_mm);
        self->card_size_mm=guide?game->species_best_size_mm[index]:game->last_size_mm;
        self->card_weight_g=guide && known?fish_species_weight_g(fish_species_profile(game->species_best_map[index],index),self->card_size_mm):game->last_weight_g;
        /* Prepare once per frame, reuse the operation across all PFB blocks. */
        self->portrait_scale_q8=known?fish_species_scale_q8(fish_species_profile(guide?game->species_best_map[index]:game->last_map,index),self->card_size_mm):256;
        if (known) {
            const fish_species_art_t *art=fish_species_portrait_art(index,self->portrait_scale_q8);
            arm_2d_point_float_t pivot={art->sprite->tRegion.tSize.iWidth/2.0f,art->sprite->tRegion.tSize.iHeight/2.0f};
            float scale=self->portrait_scale_q8/256.0f*96/art->sprite->tRegion.tSize.iWidth;
            bool mirror=fish_smoke_test==4 && self->smoke_frame%3==1;
            arm_2d_err_t result=arm_2dp_rgb565_tile_transform_xy_with_src_mask_prepare(
                &self->portrait_operation,art->sprite,art->mask,pivot,0.0f,mirror?-scale:scale,scale);
            if (fish_smoke_test && result<0) { fprintf(stderr,"Transform prepare failed: %d\n",result); exit(1); }
        }
    }
}

static IMPL_PFB_ON_DRAW(draw_scene)
{
    fish_scene_t *self=(fish_scene_t *)pTarget;
    const fish_game_t *game=&self->game;
    (void)bIsNewFrame;
    if(fish_smoke_test==35) {
        static const fish_media_status_t examples[]={
            {.phase=FISH_MEDIA_CHECK,.progress=25,.elapsed_ms=3000},
            {.phase=FISH_MEDIA_WAIT},
            {.phase=FISH_MEDIA_WAIT,.received_bytes=256000,.receiving=true,.elapsed_ms=7600},
            {.phase=FISH_MEDIA_VERIFY,.progress=65,.map=1,.elapsed_ms=12000},
            {.phase=FISH_MEDIA_SETTLE,.progress=100,.elapsed_ms=15000},
            {.phase=FISH_MEDIA_WAIT,.error=FISH_RES_ERR_MISSING,.map=1},
            {.phase=FISH_MEDIA_WAIT,.error=FISH_RES_ERR_VERSION},
            {.phase=FISH_MEDIA_FAULT,.watchdog_reset=true,.reset_stage=8,
             .fault={.detail=0x70040,.cause=6,.pc=0x80045100,.value=0xdeadbeef}}};
        media_page(ptTile,examples[self->smoke_frame]);return arm_fsm_rt_cpl;
    }
    if(game->mode==FISH_ROUTE) {route_page(ptTile,game,&self->route_view);return arm_fsm_rt_cpl;}
    if(game->mode==FISH_LOADING || !fish_resources_valid()) {loading_page(ptTile,self);return arm_fsm_rt_cpl;}
    if(fish_smoke_test==28 || (fish_smoke_test==32 && self->smoke_frame>=36)) {landscape(ptTile,game);return arm_fsm_rt_cpl;}
    if (game->mode==FISH_BAG || game->mode==FISH_SHOP || game->mode==FISH_HOLD) box(ptTile,0,0,240,240,RPG_DARK);
    else {
        landscape(ptTile,game);night_moon(ptTile,game);weather_haze(ptTile,game);
        banks_and_docks(ptTile,game);weather_rain(ptTile,game);fisherman(ptTile,game);
        hanging_fish(ptTile,self);
        foreground(ptTile,game);
        unsigned night=fish_game_night_opacity(game);
        if(night) {
            arm_2d_rgb565_fill_colour_with_opacity(ptTile,NULL,(arm_2d_color_rgb565_t){.tValue=RGB(10,18,43)},(uint8_t)(game->map_index?night/5:night));
            ARM_2D_OP_WAIT_ASYNC();
        }
        night_road(ptTile,game);station_lights(ptTile,game);
    }
    if(game->mode!=FISH_BAG && game->mode!=FISH_SHOP && game->mode!=FISH_HOLD) {
        night_float(ptTile,game);
        if(game->mode==FISH_SHORE && game->shop_closed_notice) {
            int x=game->shore_x-fish_game_camera(game);
            int y=FISH_GROUND_Y;
            if(x<23) x=23;
            if(x>217) x=217;
            box(ptTile,x-23,y-57,46,19,RPG_DARK);
            caption(ptTile,x,y-57,"打烊了",RPG_LIGHT,RPG_DARK);
        }
    }
    hud(ptTile,self); bag_panel(ptTile,self);
    shop_panel(ptTile,self);hold_panel(ptTile,self);
    money_feedback(ptTile,game);
    catch_card(ptTile,self,fish_smoke_test==4 && self->smoke_frame%3==1);
    const char *save_label=fish_save_label(&self->save);
    if(*save_label) {
        uint16_t bg=game->mode==FISH_BAG?RPG_DARK:INK;
        bool detail=self->save.error_code && (self->save.status==FISH_SAVE_ERROR || self->save.status==FISH_SAVE_INVALID);
        unsigned wire=detail?fish_storage_write_diagnostic():0;
        box(ptTile,157,0,83,wire?46:detail?38:24,bg);
        chinese_text_tinted(ptTile,202,1,save_label,self->save_box,
            self->save.status==FISH_SAVE_ERROR || self->save.status==FISH_SAVE_INVALID?RED:SUN,bg);
        if(detail) {
            char code[16];snprintf(code,sizeof(code),"S%02u/%02u",self->save.error_code>>8,self->save.error_code&255);
            text(ptTile,172,25,RED,code);
            if(wire){snprintf(code,sizeof(code),"W%02u/%02X",wire>>8,wire&255);text(ptTile,172,35,RED,code);}
        }
    }
    ARM_2D_OP_WAIT_ASYNC();
    return arm_fsm_rt_cpl;
}

void fish_scene_init(arm_2d_scene_player_t *player)
{
    fish_game_init(&scene.game,fish_hpm_millis());
    fish_hpm_feedback_init();
    if(!fish_smoke_test) fish_save_init(&scene.save,&scene.game,scene.game.now);
    if(fish_smoke_test) {
        fish_resources_begin(0);for(unsigned i=0;i<1000 && fish_resources_phase()!=FISH_RES_DONE && fish_resources_phase()!=FISH_RES_ERROR;i++)fish_resources_tick();
        if(!fish_resources_valid()) {fputs("Missing/invalid simulator SD packs\n",stderr);exit(1);}
    }
    ARM_2D_OP_INIT(scene.portrait_operation);
    ARM_2D_OP_INIT(scene.backpack_operation);
    ARM_2D_OP_INIT(scene.exterior_closed_operation);
    ARM_2D_OP_INIT(scene.shop_detail_operation);
    ARM_2D_OP_INIT(scene.hanging_operation);
    ARM_2D_OP_INIT(scene.container_operation);
    ARM_2D_OP_INIT(scene.footpath_operation);
    ARM_2D_OP_INIT(scene.exterior_operation);
    ARM_2D_OP_INIT(scene.station_operation);
    ARM_2D_OP_INIT(scene.shore_player_operation);
    for(unsigned side=0;side<2;side++) ARM_2D_OP_INIT(scene.dock_operation[side]);
    ARM_2D_OP_INIT(scene.hold_portrait_operation);
    ARM_2D_OP_INIT(scene.park_lamp_operation);
    ARM_2D_OP_INIT(scene.park_light_operation);
    ARM_2D_OP_INIT(scene.cloud_operation);
    scene.use_as__arm_2d_scene_t=(arm_2d_scene_t){ .tCanvas={SKY}, .fnScene=draw_scene,
        .fnOnFrameStart=frame_start, .fnOnFrameCPL=NULL,
        .bUseDirtyRegionHelper=false };
    arm_2d_scene_player_append_scenes(player,&scene.use_as__arm_2d_scene_t,1);
}

void fish_scene_memory_layout(void)
{
    printf("state_bytes=%zu scene_bytes=%zu transform_bytes=%zu species_record_bytes=%zu catalog_bytes=%zu\n",
        sizeof(fish_game_t),sizeof(fish_scene_t),sizeof(arm_2d_op_trans_msk_t),sizeof(fish_species_t),sizeof(fish_species));
}

void fish_scene_input(fish_event_t event,int steps,uint32_t timestamp)
{
    if(fish_media_phase()==FISH_MEDIA_FAULT) {
        if(event==FISH_RELEASE)fish_media_retry(timestamp);
        return;
    }
    fish_game_input(&scene.game,event,steps,timestamp);
    fish_hpm_feedback_update(&scene.game);
}
