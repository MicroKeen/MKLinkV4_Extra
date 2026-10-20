#ifndef TEST_FF_H
#define TEST_FF_H
#include <stddef.h>
typedef struct {int unused;} FATFS;
#define FR_OK 0
int f_mount(FATFS *,const char *,int);
#endif
