#ifndef FISH_SAVE_CRYPTO_H
#define FISH_SAVE_CRYPTO_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* RFC 5297 AES-SIV-CMAC-256, one AD string; output may overlap plaintext. */
void fish_save_seal(const uint8_t key[32],const uint8_t *ad,size_t ad_len,
    const uint8_t *plain,size_t len,uint8_t tag[16],uint8_t *cipher);
bool fish_save_open(const uint8_t key[32],const uint8_t *ad,size_t ad_len,
    const uint8_t tag[16],const uint8_t *cipher,size_t len,uint8_t *plain);
void fish_save_device_key(const uint8_t uuid[16],uint8_t key[32]);
#endif
