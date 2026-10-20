#ifndef FISH_FS_HPM_H
#define FISH_FS_HPM_H
int fish_fs_lock(void);
int fish_fs_resource_lock(void);
void fish_fs_unlock(void);
int fish_fs_usb_begin(void);
void fish_fs_usb_end(void);
int fish_fs_usb_probe(int enabled);
#endif
