#include <cstring>
#include "ff.h"
#include "diskio.h"


#include "hardware/flash.h"
#include "pico/stdlib.h"
#include "hardware/sync.h"


#define FATFS_OFFSET      (2 * 1024 * 1024 - 128 * 1024)  // Last 128 KB (for a 2MB flash)
#define FATFS_SIZE        (128 * 1024)                    // 128 KB
#define FATFS_SECTOR_SIZE 512                             // FAT sector logic
#define FATFS_SECTOR_COUNT (FATFS_SIZE / FATFS_SECTOR_SIZE)

bool disk_init(); 