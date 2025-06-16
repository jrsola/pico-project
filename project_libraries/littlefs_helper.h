#include "lfs.h"
#include "hardware/flash.h"

extern lfs_t lfs;
extern const struct lfs_config cfg;

int pico_mount(bool format);  // Your custom implementation