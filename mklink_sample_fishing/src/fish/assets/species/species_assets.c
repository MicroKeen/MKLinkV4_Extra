#include "species_assets.h"
const fish_species_art_t fish_species_art[FISH_SPECIES_COUNT]={
    {&c_tileSpeciesCrucianRGB565,&c_tileSpeciesCrucianAlphaGRAY8},
    {&c_tileSpeciesCarpRGB565,&c_tileSpeciesCarpAlphaGRAY8},
    {&c_tileSpeciesHooksnoutRGB565,&c_tileSpeciesHooksnoutAlphaGRAY8},
    {&c_tileSpeciesCatfishRGB565,&c_tileSpeciesCatfishAlphaGRAY8},
    {&c_tileSpeciesGobyRGB565,&c_tileSpeciesGobyAlphaGRAY8},
    {&c_tileSpeciesSnakeheadRGB565,&c_tileSpeciesSnakeheadAlphaGRAY8},
    {&c_tileSpeciesYellowCatfishRGB565,&c_tileSpeciesYellowCatfishAlphaGRAY8},
    {&c_tileSpeciesSharpbellyRGB565,&c_tileSpeciesSharpbellyAlphaGRAY8},
    {&c_tileSpeciesMandarinRGB565,&c_tileSpeciesMandarinAlphaGRAY8},
    {&c_tileSpeciesSoftshellRGB565,&c_tileSpeciesSoftshellAlphaGRAY8},
    {&c_tileSpeciesHenanCrabRGB565,&c_tileSpeciesHenanCrabAlphaGRAY8},
    {&c_tileSpeciesCatfishXanthicRGB565,&c_tileSpeciesCatfishXanthicAlphaGRAY8},
    {&c_tileSpeciesHooksnoutLongfinRGB565,&c_tileSpeciesHooksnoutLongfinAlphaGRAY8},
    {&c_tileSpeciesCarpRedLongfinRGB565,&c_tileSpeciesCarpRedLongfinAlphaGRAY8},
    {&c_tileSpeciesSoftshellAlbinoRGB565,&c_tileSpeciesSoftshellAlbinoAlphaGRAY8},
};
const fish_species_art_t fish_species_large_art[FISH_SPECIES_COUNT]={
    [1]={&c_tileSpeciesCarpLargeRGB565,&c_tileSpeciesCarpLargeAlphaGRAY8},
    [3]={&c_tileSpeciesCatfishLargeRGB565,&c_tileSpeciesCatfishLargeAlphaGRAY8},
    [5]={&c_tileSpeciesSnakeheadLargeRGB565,&c_tileSpeciesSnakeheadLargeAlphaGRAY8},
};
const fish_species_art_t *fish_species_portrait_art(unsigned index,uint16_t scale_q8)
{
    return scale_q8>256 && fish_species_large_art[index].sprite?&fish_species_large_art[index]:&fish_species_art[index];
}
