/* Published with the author's explicit permission for legacy save compatibility.
 * Keep these bytes stable. The public key does not provide strong anti-cheat. */
#include "fish_save_crypto.h"
#include "tinycrypt/cmac_mode.h"
#include <string.h>
static const uint8_t master[32]={0xe2,0xbb,0xcc,0xd5,0x1c,0xbf,0x25,0xad,0x10,0xc2,0x20,0xf9,0x57,0x29,0x81,0x84,0x03,0x05,0x23,0x4d,0xd9,0x6c,0x68,0x56,0x16,0x60,0x0d,0x2c,0x06,0x66,0x66,0xb9};
void fish_save_device_key(const uint8_t uuid[16],uint8_t key[32])
{
    for(unsigned half=0;half<2;half++) {
        struct tc_aes_key_sched_struct sched;
        struct tc_cmac_struct state;
        uint8_t domain=(uint8_t)half;
        tc_cmac_setup(&state,master+16*half,&sched);
        tc_cmac_update(&state,(const uint8_t *)"fish-save-v1",12);
        tc_cmac_update(&state,&domain,1);tc_cmac_update(&state,uuid,16);
        tc_cmac_final(key+16*half,&state);
        volatile uint8_t *b=(volatile uint8_t *)&sched;
        for(unsigned i=0;i<sizeof(sched);i++) b[i]=0;
    }
}
