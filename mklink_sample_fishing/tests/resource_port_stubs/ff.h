typedef unsigned UINT;
typedef int FRESULT;
typedef struct {unsigned size;} FIL;
enum {FR_OK,FR_NO_FILE,FR_NO_PATH,FR_DISK_ERR,FA_READ};
#define f_size(f) ((f)->size)
FRESULT f_open(FIL *,const char *,unsigned);
FRESULT f_lseek(FIL *,unsigned);
FRESULT f_read(FIL *,void *,unsigned,UINT *);
FRESULT f_close(FIL *);
