#ifndef FISH_CITY_VENDOR_INDEXED_H
#define FISH_CITY_VENDOR_INDEXED_H
#include <stdint.h>
typedef struct {const uint16_t *palette;const uint8_t *indices;uint8_t width,height,bits;} fish_vendor_image_t;
extern const fish_vendor_image_t fish_image_MerchantGreeting;
extern const fish_vendor_image_t fish_image_MerchantSold;
extern const fish_vendor_image_t fish_image_MerchantBought;
extern const fish_vendor_image_t fish_image_MerchantSurprised;
extern const fish_vendor_image_t fish_image_CityClerkGreeting;
extern const fish_vendor_image_t fish_image_CityClerkConcerned;
extern const fish_vendor_image_t fish_image_CityClerkLatte;
extern const fish_vendor_image_t fish_image_CityStallExterior;
extern const fish_vendor_image_t fish_image_CityStallInterior;
#endif
