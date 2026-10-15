#include "fish_save.h"
#include "fish_save_crypto.h"
#include "fish_save_storage.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "save_long_vector.h"

static uint8_t slots[2][FISH_SAVE_FILE_BYTES],queued[FISH_SAVE_FILE_BYTES];
static int exists[2],read_error[2],write_result=1;
static unsigned queued_slot,writes;
static size_t slot_lengths[2];
static const uint8_t key[32]={1,2,3};
int fish_storage_init(uint8_t k[32]) {memcpy(k,key,32);return 0;}
int fish_storage_read(unsigned slot,uint8_t *file,size_t len)
{
    if(read_error[slot]) return read_error[slot];
    if(!exists[slot]) return -2;
    if(len!=slot_lengths[slot]) return -3;
    memcpy(file,slots[slot],len);return 0;
}
int fish_storage_begin(unsigned slot,const uint8_t *file,size_t len)
{
    queued_slot=slot;memcpy(queued,file,len);++writes;return 0;
}
int fish_storage_poll(void)
{
    if(write_result>0) {memcpy(slots[queued_slot],queued,sizeof(queued));exists[queued_slot]=1;slot_lengths[queued_slot]=sizeof(queued);}
    return write_result;
}
static void reset(void)
{
    memset(slots,0,sizeof(slots));memset(exists,0,sizeof(exists));memset(read_error,0,sizeof(read_error));
    writes=0;write_result=1;
    slot_lengths[0]=slot_lengths[1]=FISH_SAVE_FILE_BYTES;
}
static void caught(fish_game_t *g)
{
    g->catches=3;g->last_species=1;g->last_size_mm=fish_species[1].typical_min_mm;
    g->species_catches[1]=3;g->species_best_size_mm[1]=g->last_size_mm;
    g->rod_index=2;fish_game_equip_line(g,2);g->bait_index=4;g->float_index=2;g->depth_index=0;
    g->economy.rods=g->economy.lines=g->economy.floats=7;g->economy.coins=123;
    for(unsigned i=0;i<FISH_SPECIAL_BAIT_COUNT;i++) g->economy.bait_stock[i]=5;
    fish_hold_add(&g->economy,1,g->last_size_mm,fish_species_weight_g(&fish_species[1],g->last_size_mm));
    fish_hold_add(&g->economy,1,1000,fish_species_weight_g(&fish_species[1],1000));g->game_minute=1439;
}
static void manual(fish_save_t *s,fish_game_t *g,uint32_t now)
{
    g->save_requested=true;fish_save_tick(s,g,now);assert(s->writing && s->status==FISH_SAVE_SAVING);
    fish_save_tick(s,g,now+10);assert(!s->writing && s->status==FISH_SAVE_OK);
}
int main(void)
{
    /* RFC 5297 Appendix A.1, including the short-plaintext S2V branch. */
    uint8_t vk[32]={0xff,0xfe,0xfd,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,0xf6,0xf5,0xf4,0xf3,0xf2,0xf1,0xf0,
        0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff};
    uint8_t ad[24],plain[14]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee};
    for(unsigned i=0;i<24;i++) ad[i]=(uint8_t)(0x10+i);
    const uint8_t expected_tag[16]={0x85,0x63,0x2d,0x07,0xc6,0xe8,0xf3,0x7f,0x95,0x0a,0xcd,0x32,0x0a,0x2e,0xcc,0x93};
    const uint8_t expected_cipher[14]={0x40,0xc0,0x2b,0x96,0x90,0xc4,0xdc,0x04,0xda,0xef,0x7f,0x6a,0xfe,0x5c};
    uint8_t tag[16],cipher[14],opened[14];
    fish_save_seal(vk,ad,24,plain,14,tag,cipher);
    assert(!memcmp(tag,expected_tag,16) && !memcmp(cipher,expected_cipher,14));
    assert(fish_save_open(vk,ad,24,tag,cipher,14,opened) && !memcmp(plain,opened,14));
    uint8_t long_key[32],long_ad[16],long_plain[60],long_cipher[60];
    for(unsigned i=0;i<32;i++) long_key[i]=(uint8_t)i;
    for(unsigned i=0;i<16;i++) long_ad[i]=(uint8_t)i;
    for(unsigned i=0;i<60;i++) long_plain[i]=(uint8_t)i;
    fish_save_seal(long_key,long_ad,16,long_plain,60,tag,long_cipher);
    assert(!memcmp(tag,long_expected,16) && !memcmp(long_cipher,long_expected+16,60));

    fish_game_t g,loaded,unchanged;fish_save_t s;
    uint8_t payload[FISH_SAVE_PAYLOAD_BYTES],output[FISH_SAVE_PAYLOAD_BYTES],file[FISH_SAVE_FILE_BYTES];
    fish_game_init(&g,10);caught(&g);g.economy.container=2;fish_save_pack(&g,payload);fish_save_encode(key,5,payload,file);
    uint64_t seq=0;assert(fish_save_decode(key,file,sizeof(file),&seq,output) && seq==5 && !memcmp(payload,output,sizeof(payload)));
    /* Change every single byte: header, sequence, MAC, encrypted progress. */
    for(unsigned i=0;i<sizeof(file);i++) {
        file[i]^=1;assert(!fish_save_decode(key,file,sizeof(file),&seq,output));file[i]^=1;
    }
    for(unsigned n=0;n<sizeof(file);n++) assert(!fish_save_decode(key,file,n,&seq,output));
    assert(!fish_save_decode(vk,file,sizeof(file),&seq,output));
    fish_game_init(&loaded,99);assert(fish_save_unpack(&loaded,payload));
    assert(loaded.economy.coins==123 && fish_hold_weight(&loaded.economy)==fish_hold_weight(&g.economy));
    assert(loaded.economy.container==2 && fish_hold_capacity(&loaded.economy)==35000);
    assert(loaded.game_minute==1439 && loaded.economy.hanging_species==1 && loaded.economy.hanging_size_mm==1000 &&
           loaded.economy.hanging_value==g.economy.hanging_value && loaded.economy.bait_stock[4]==5);
    assert(loaded.mode==FISH_MOVE && !loaded.pressed && loaded.catches==3 && loaded.line_strength==140 && loaded.bait_index==4);
    unchanged=loaded;payload[0]=255;assert(!fish_save_unpack(&loaded,payload) && !memcmp(&loaded,&unchanged,sizeof(loaded)));fish_save_pack(&g,payload);
    payload[18+4]=0xff;payload[19+4]=0xff;assert(!fish_save_unpack(&loaded,payload));
    fish_save_pack(&g,payload);unchanged=loaded;payload[200]=FISH_CONTAINER_COUNT;
    assert(!fish_save_unpack(&loaded,payload) && !memcmp(&loaded,&unchanged,sizeof(loaded)));
    /* A real v2 file has zero reserved bytes at 200..203 and gains a small
     * bucket on migration; stored cargo and currency stay exactly unchanged. */
    reset();fish_game_init(&g,0);caught(&g);fish_save_pack(&g,payload);payload[201]=0;
    memcpy(file,"FSG1",4);file[4]=2;file[5]=0;file[6]=FISH_SAVE_PAYLOAD_BYTES;file[7]=0;
    memset(file+8,0,8);file[8]=9;
    fish_save_seal(key,file,16,payload,FISH_SAVE_PAYLOAD_BYTES,file+16,file+32);
    memcpy(slots[0],file,sizeof(file));exists[0]=1;
    fish_game_init(&loaded,0);fish_save_init(&s,&loaded,0);
    assert(s.sequence==9 && loaded.save_requested && loaded.economy.container==0 && loaded.economy.coins==123);
    assert(loaded.economy.hanging_species==1 && fish_hold_weight(&loaded.economy)==fish_hold_weight(&g.economy));
    assert(loaded.rod_index==0 && loaded.line_index==1 && loaded.float_index==0 && loaded.bait_index==5);
    manual(&s,&loaded,1);assert(s.sequence==10 && slots[1][4]==4 && !memcmp(slots[0],file,sizeof(file)));
    /* Authenticated v3: reset equipment once, preserve every other field,
     * then allow newly purchased equipment to survive the next boot. */
    reset();g.economy.container=2;fish_save_pack(&g,payload);payload[201]=0;
    file[4]=3;file[8]=11;
    fish_save_seal(key,file,16,payload,FISH_SAVE_PAYLOAD_BYTES,file+16,file+32);
    memcpy(slots[0],file,sizeof(file));exists[0]=1;
    fish_game_init(&loaded,0);fish_save_init(&s,&loaded,0);
    assert(s.sequence==11 && loaded.save_requested && loaded.rod_index==0 && loaded.line_index==1);
    assert(loaded.float_index==0 && loaded.bait_index==5 && loaded.depth_index==FISH_DEPTH_OPTION_COUNT-1);
    assert(loaded.economy.rods==1 && loaded.economy.lines==2 && loaded.economy.floats==1);
    assert(!memcmp(loaded.species_catches,g.species_catches,sizeof(g.species_catches)) &&
           !memcmp(loaded.species_best_size_mm,g.species_best_size_mm,sizeof(g.species_best_size_mm)));
    assert(loaded.catches==g.catches && loaded.last_species==g.last_species && loaded.last_size_mm==g.last_size_mm);
    fish_economy_t preserved=g.economy;preserved.rods=1;preserved.lines=2;preserved.floats=1;
    assert(!memcmp(&loaded.economy,&preserved,sizeof(preserved)) && loaded.game_minute==g.game_minute);
    manual(&s,&loaded,1);assert(s.sequence==12 && slots[1][4]==4 && !memcmp(slots[0],file,sizeof(file)));
    /* Power-cycle after migration and after acquiring new gear. */
    fish_game_init(&loaded,0);fish_save_init(&s,&loaded,0);assert(!loaded.save_requested && loaded.rod_index==0);
    loaded.economy.rods=7;loaded.economy.lines=7;loaded.economy.floats=7;
    loaded.rod_index=2;fish_game_equip_line(&loaded,2);loaded.float_index=2;
    manual(&s,&loaded,2);
    fish_game_init(&loaded,0);fish_save_init(&s,&loaded,0);
    assert(loaded.rod_index==2 && loaded.line_index==2 && loaded.float_index==2 && !loaded.save_requested);
    /* A valid v4 slot prevents another reset after older firmware writes v3. */
    file[8]=20;fish_save_seal(key,file,16,payload,FISH_SAVE_PAYLOAD_BYTES,file+16,file+32);
    memcpy(slots[0],file,sizeof(file));exists[0]=1;
    fish_game_init(&loaded,0);fish_save_init(&s,&loaded,0);
    assert(s.sequence==20 && loaded.rod_index==2 && loaded.economy.rods==7 && loaded.save_requested);
    /* v4 cannot authenticate an uninitialized marker as an ordinary save. */
    file[4]=4;fish_save_seal(key,file,16,payload,FISH_SAVE_PAYLOAD_BYTES,file+16,file+32);
    assert(!fish_save_decode(key,file,sizeof(file),&seq,output));
    /* Actual authenticated v1 file: exact 92 bytes, same key/indices/sequence.
     * Boot schedules v4 in inactive slot; old active file survives failed write. */
    reset();fish_save_pack(&g,payload);
    uint8_t legacy[FISH_SAVE_LEGACY_FILE_BYTES]={0};memcpy(legacy,"FSG1",4);
    legacy[4]=1;legacy[6]=FISH_SAVE_LEGACY_PAYLOAD_BYTES;legacy[8]=7;
    fish_save_seal(key,legacy,16,payload,FISH_SAVE_LEGACY_PAYLOAD_BYTES,legacy+16,legacy+32);
    memcpy(slots[0],legacy,sizeof(legacy));slot_lengths[0]=sizeof(legacy);exists[0]=1;
    fish_game_init(&loaded,0);fish_save_init(&s,&loaded,0);
    assert(s.active_slot==0 && s.sequence==7 && loaded.catches==3 && loaded.save_requested);
    assert(loaded.rod_index==0 && loaded.bait_index==5 && loaded.economy.bait_stock[4]==5 && loaded.economy.coins==0);
    write_result=-1;fish_save_tick(&s,&loaded,1);fish_save_tick(&s,&loaded,2);
    assert(s.sequence==7 && !memcmp(slots[0],legacy,sizeof(legacy)));
    write_result=1;manual(&s,&loaded,3);assert(s.sequence==8 && s.active_slot==1);
    fish_game_init(&loaded,9);fish_save_init(&s,&loaded,9);
    assert(s.sequence==8 && loaded.catches==3 && loaded.economy.rods==1);

    reset();fish_game_init(&g,0);fish_save_init(&s,&g,0);caught(&g);
    fish_save_tick(&s,&g,299999);assert(!writes);
    fish_save_tick(&s,&g,300000);assert(writes==1 && s.status==FISH_SAVE_SAVING);
    g.rod_index=1;fish_save_tick(&s,&g,300010);assert(s.sequence==1 && s.saved[0]==2);
    fish_save_tick(&s,&g,600000);assert(writes==2);fish_save_tick(&s,&g,600010);
    fish_save_tick(&s,&g,900000);assert(writes==2); /* No changes: skip wear. */
    manual(&s,&g,900020);assert(s.sequence==3);
    fish_game_init(&loaded,42);fish_save_init(&s,&loaded,42);
    assert(s.sequence==3 && loaded.rod_index==1 && loaded.species_catches[1]==3 && loaded.last_weight_g>0);
    slots[s.active_slot][40]^=1;fish_game_init(&loaded,42);fish_save_init(&s,&loaded,42);
    assert(s.status==FISH_SAVE_RECOVERED && s.sequence==2 && loaded.catches==3);
    /* Torn inactive file cannot destroy the latest good one. */
    unsigned inactive=s.active_slot==0?1:0;memset(slots[inactive],0,sizeof(slots[inactive]));
    fish_game_init(&loaded,50);fish_save_init(&s,&loaded,50);assert(loaded.catches==3 && s.sequence==2);
    loaded.rod_index=0;write_result=-1;loaded.save_requested=true;fish_save_tick(&s,&loaded,60);fish_save_tick(&s,&loaded,70);
    assert(s.status==FISH_SAVE_ERROR && s.sequence==2 && s.saved[0]==1);
    write_result=1;manual(&s,&loaded,80);assert(s.sequence==3);
    /* A press during writing is retained and captures any later change. */
    loaded.save_requested=true;fish_save_tick(&s,&loaded,85);assert(s.writing);
    loaded.rod_index=2;loaded.save_requested=true;fish_save_tick(&s,&loaded,86);
    assert(s.manual_pending && s.saved[0]==0);
    fish_save_tick(&s,&loaded,87);assert(s.writing);fish_save_tick(&s,&loaded,88);assert(s.saved[0]==2);
    /* Both invalid: don't silently erase files or load the modified records. */
    memset(slots,0,sizeof(slots));fish_game_init(&loaded,90);fish_save_init(&s,&loaded,90);
    assert(s.blocked && s.status==FISH_SAVE_INVALID && loaded.catches==0);
    unsigned before=writes;loaded.save_requested=true;fish_save_tick(&s,&loaded,100);assert(writes==before);
    reset();read_error[0]=-1;fish_game_init(&loaded,0);fish_save_init(&s,&loaded,0);
    assert(s.blocked && s.status==FISH_SAVE_ERROR);
    loaded.save_requested=true;fish_save_tick(&s,&loaded,1);assert(!writes && s.status==FISH_SAVE_ERROR);
    /* Timer wrap remains a full five-minute interval. */
    reset();fish_game_init(&g,UINT32_MAX-100);fish_save_init(&s,&g,UINT32_MAX-100);caught(&g);
    fish_save_tick(&s,&g,299898);assert(!writes);fish_save_tick(&s,&g,299899);assert(writes==1);
    /* UI action sets a one-shot request and stays in the bag. */
    fish_game_init(&g,0);g.mode=FISH_BAG;g.bag_index=FISH_BAG_SAVE;
    fish_game_input(&g,FISH_PRESS,0,1);fish_game_input(&g,FISH_RELEASE,0,2);
    assert(g.save_requested && g.mode==FISH_BAG && !g.bag_editing);
    uint8_t uuid[16]={0},k1[32],k2[32];fish_save_device_key(uuid,k1);uuid[0]=1;fish_save_device_key(uuid,k2);assert(memcmp(k1,k2,32));
    /* Compute benchmark; excludes SD/filesystem latency and debug IO. */
    fish_game_init(&g,0);caught(&g);fish_save_pack(&g,payload);
    clock_t start=clock();for(unsigned i=0;i<10000;i++) fish_save_encode(key,i+1,payload,file);
    double us=(double)(clock()-start)*1000000.0/CLOCKS_PER_SEC/10000;
    printf("PASS: SIV vector, tamper/truncate/key rejection, restore, five-minute autosave, wrap, A/B recovery, failure/retry; file=%u bytes; PC encryption %.1f us/save\n",FISH_SAVE_FILE_BYTES,us);
    return 0;
}
