#define HPM_L1C_CACHELINE_ALIGN_DOWN(x) ((x)&~31u)
#define HPM_L1C_CACHELINE_ALIGN_UP(x) (((x)+31u)&~31u)
void l1c_dc_invalidate(uint32_t,unsigned);
