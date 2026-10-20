#include <stdint.h>
#define OTP_SOC_UUID_IDX 0
static inline void otp_init(void) {}
static inline uint32_t otp_read_from_shadow(unsigned index) {return 0x12345678u+index;}
