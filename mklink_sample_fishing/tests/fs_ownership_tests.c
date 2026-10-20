#include "fish_fs_hpm.h"
#include "rtthread.h"
#include "ff.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned nesting,inits,mounts,unmounts,sd_inits;
static int mount_error;
int rt_mutex_init(struct rt_mutex *m,const char *name,int flags)
{(void)m;assert(!strcmp(name,"fishfs") && flags==RT_IPC_FLAG_PRIO);++inits;return 0;}
int rt_mutex_take(struct rt_mutex *m,int wait)
{(void)m;assert(wait==RT_WAITING_FOREVER);++nesting;return 0;}
int rt_mutex_release(struct rt_mutex *m) {(void)m;assert(nesting);--nesting;return 0;}
int spi_sd_init(void) {++sd_inits;return 0;}
int disk_initialize(unsigned drive) {assert(drive==3);return 0;}
int f_mount(FATFS *fs,const char *drive,int immediate)
{
    assert(nesting && !strcmp(drive,"3:"));
    if(fs) {assert(immediate==1);++mounts;return mount_error;}
    assert(immediate==0);++unmounts;return 0;
}
int main(void)
{
    assert(!fish_fs_lock());assert(!fish_fs_lock());fish_fs_unlock();fish_fs_unlock();
    assert(inits==1 && sd_inits==1 && mounts==1 && !nesting);
    assert(!fish_fs_usb_begin() && unmounts==1 && !nesting);
    assert(fish_fs_lock() && fish_fs_resource_lock());assert(!nesting && mounts==1);
    assert(!fish_fs_usb_probe(1) && mounts==2);
    assert(fish_fs_lock()); /* A read-only probe never grants save access. */
    assert(!fish_fs_resource_lock());fish_fs_unlock();assert(!nesting);
    assert(!fish_fs_usb_probe(0) && unmounts==2);
    assert(fish_fs_resource_lock() && !nesting);
    fish_fs_usb_end();assert(!fish_fs_lock());fish_fs_unlock();
    assert(mounts==3 && sd_inits==1 && !nesting);
    assert(!fish_fs_usb_begin());mount_error=1;
    assert(fish_fs_usb_probe(1));assert(fish_fs_resource_lock());assert(!nesting);
    mount_error=0;assert(!fish_fs_usb_probe(1));
    assert(!fish_fs_resource_lock());fish_fs_unlock();
    fish_fs_usb_probe(0);fish_fs_usb_end();
    assert(!fish_fs_lock());fish_fs_unlock();assert(!nesting);
    puts("PASS: real filesystem ownership gate; recursive save lock; USB unmount; read-only probes; blocked saves; failed mounts and remount recovery");
}
