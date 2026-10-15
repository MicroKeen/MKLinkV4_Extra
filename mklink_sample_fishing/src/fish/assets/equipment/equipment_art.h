#ifndef RPG_EQUIPMENT_ART_H
#define RPG_EQUIPMENT_ART_H
#include "arm_2d.h"
enum {RPG_EQUIPMENT_ART_COUNT=16,RPG_LINE_ART=3,RPG_FLOAT_ART=6,RPG_BAIT_ART=9,RPG_DEPTH_ART=14,RPG_SATCHEL_ART=15};
typedef struct {const arm_2d_tile_t *sprite,*mask;} fish_ui_art_t;
extern const fish_ui_art_t fish_equipment_art[RPG_EQUIPMENT_ART_COUNT];
extern const arm_2d_tile_t c_tileRPGBambooRodRGB565;
extern const arm_2d_tile_t c_tileRPGBambooRodAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGFiberglassRodRGB565;
extern const arm_2d_tile_t c_tileRPGFiberglassRodAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGCarbonRodRGB565;
extern const arm_2d_tile_t c_tileRPGCarbonRodAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGLightLineRGB565;
extern const arm_2d_tile_t c_tileRPGLightLineAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGStandardLineRGB565;
extern const arm_2d_tile_t c_tileRPGStandardLineAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGHeavyLineRGB565;
extern const arm_2d_tile_t c_tileRPGHeavyLineAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGRedWhiteFloatRGB565;
extern const arm_2d_tile_t c_tileRPGRedWhiteFloatAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGFeatherFloatRGB565;
extern const arm_2d_tile_t c_tileRPGFeatherFloatAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGGlowFloatRGB565;
extern const arm_2d_tile_t c_tileRPGGlowFloatAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGWormRGB565;
extern const arm_2d_tile_t c_tileRPGWormAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGRiceRGB565;
extern const arm_2d_tile_t c_tileRPGRiceAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGDoughRGB565;
extern const arm_2d_tile_t c_tileRPGDoughAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGSmallFishRGB565;
extern const arm_2d_tile_t c_tileRPGSmallFishAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGShrimpRGB565;
extern const arm_2d_tile_t c_tileRPGShrimpAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGDepthRGB565;
extern const arm_2d_tile_t c_tileRPGDepthAlphaGRAY8;
extern const arm_2d_tile_t c_tileRPGSatchelRGB565;
extern const arm_2d_tile_t c_tileRPGSatchelAlphaGRAY8;
#endif
