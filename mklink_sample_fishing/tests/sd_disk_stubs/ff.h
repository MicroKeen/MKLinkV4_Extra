#ifndef TEST_FF_H
#define TEST_FF_H
#include <stddef.h>
#include <stdint.h>
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef unsigned UINT;
/* Exercise large LBAs without narrowing them before the driver guard. */
typedef uint64_t LBA_t;
#endif
