/* Test the exact board diskio module underneath a simulated host drive 0.
 * These renames apply only to this translation unit, never to production. */
#define disk_read test_board_disk_read
#define disk_write test_board_disk_write
#define disk_ioctl test_board_disk_ioctl
#define disk_initialize test_board_disk_initialize
#define disk_deinitialize test_board_disk_deinitialize
#define disk_status test_board_disk_status
#include FISH_DISKIO_SOURCE
