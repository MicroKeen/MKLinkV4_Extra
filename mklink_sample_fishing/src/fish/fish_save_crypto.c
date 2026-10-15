#include "fish_save_crypto.h"
#include <string.h>
#include "tinycrypt/aes.h"
#include "tinycrypt/cmac_mode.h"
#include "tinycrypt/utils.h"

static void wipe(void *p,size_t n)
{
    volatile uint8_t *b=p;while(n--) *b++=0;
}
static void mac(const uint8_t key[16],const uint8_t *data,size_t len,uint8_t out[16])
{
    struct tc_aes_key_sched_struct sched;
    struct tc_cmac_struct state;
    tc_cmac_setup(&state,key,&sched);tc_cmac_update(&state,data,len);tc_cmac_final(out,&state);
    wipe(&sched,sizeof(sched));
}
static void double_block(uint8_t b[16])
{
    uint8_t carry=b[0]>>7;
    for(unsigned i=0;i<15;i++) b[i]=(uint8_t)((b[i]<<1)|(b[i+1]>>7));
    b[15]=(uint8_t)((b[15]<<1)^(0x87u*carry));
}
static void s2v(const uint8_t key[16],const uint8_t *ad,size_t ad_len,
    const uint8_t *plain,size_t len,uint8_t tag[16])
{
    uint8_t d[16]={0},t[16],last[16];
    mac(key,d,16,d);double_block(d);mac(key,ad,ad_len,t);
    for(unsigned i=0;i<16;i++) d[i]^=t[i];
    if(len>=16) {
        struct tc_aes_key_sched_struct sched;
        struct tc_cmac_struct state;
        tc_cmac_setup(&state,key,&sched);tc_cmac_update(&state,plain,len-16);
        for(unsigned i=0;i<16;i++) last[i]=plain[len-16+i]^d[i];
        tc_cmac_update(&state,last,16);tc_cmac_final(tag,&state);wipe(&sched,sizeof(sched));
    } else {
        double_block(d);memset(last,0,16);if(len) memcpy(last,plain,len);last[len]=0x80;
        for(unsigned i=0;i<16;i++) last[i]^=d[i];
        mac(key,last,16,tag);
    }
    wipe(d,16);wipe(t,16);wipe(last,16);
}
static void crypt(const uint8_t key[16],const uint8_t tag[16],const uint8_t *in,size_t len,uint8_t *out)
{
    struct tc_aes_key_sched_struct sched;
    uint8_t counter[16],stream[16];memcpy(counter,tag,16);
    counter[8]&=0x7f;counter[12]&=0x7f;
    tc_aes128_set_encrypt_key(&sched,key);
    for(size_t offset=0;offset<len;offset+=16) {
        tc_aes_encrypt(stream,counter,&sched);
        size_t n=len-offset<16?len-offset:16;
        for(size_t i=0;i<n;i++) out[offset+i]=in[offset+i]^stream[i];
        for(int i=15;i>=0;i--) if(++counter[i]) break;
    }
    wipe(&sched,sizeof(sched));wipe(stream,16);wipe(counter,16);
}
void fish_save_seal(const uint8_t key[32],const uint8_t *ad,size_t ad_len,
    const uint8_t *plain,size_t len,uint8_t tag[16],uint8_t *cipher)
{
    s2v(key,ad,ad_len,plain,len,tag);crypt(key+16,tag,plain,len,cipher);
}
bool fish_save_open(const uint8_t key[32],const uint8_t *ad,size_t ad_len,
    const uint8_t tag[16],const uint8_t *cipher,size_t len,uint8_t *plain)
{
    uint8_t computed[16];crypt(key+16,tag,cipher,len,plain);s2v(key,ad,ad_len,plain,len,computed);
    bool ok=_compare(computed,tag,16)==0;wipe(computed,16);
    if(!ok) wipe(plain,len);
    return ok;
}
