#ifndef TEST_DISKIO_H
#define TEST_DISKIO_H
#include "ff.h"
#include <stdbool.h>
typedef BYTE DSTATUS;
typedef enum {RES_OK,RES_ERROR,RES_WRPRT,RES_NOTRDY,RES_PARERR} DRESULT;
#define DEV_SD 3U
#define STA_NOINIT 1U
#define CTRL_SYNC 0U
#define GET_SECTOR_COUNT 1U
#define GET_SECTOR_SIZE 2U
#define GET_BLOCK_SIZE 3U
#define CTRL_TRIM 4U
#define CTRL_FORMAT 8U
#endif
