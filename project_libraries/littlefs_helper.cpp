#include "littlefs_helper.h"


#define FS_OFFSET (256 * 1024)  // 256KB into flash
#define FS_SIZE   (512 * 1024)  // 512KB reserved

lfs_t lfs;
const struct lfs_config cfg = {
    .read  = lfs_read,
    .prog  = lfs_prog,
    .erase = lfs_erase,
    .sync  = lfs_sync,
    .read_size = 16,
    .prog_size = 16,
    .block_size = 4096,
    .block_count = FS_SIZE / 4096,
    .block_cycles = 500,
};

int pico_mount(bool format) {
    int err = lfs_mount(&lfs, &cfg);
    if (err && format) {
        err = lfs_format(&lfs, &cfg);
        if (err) return err;
        err = lfs_mount(&lfs, &cfg);
    }
    return err;
}