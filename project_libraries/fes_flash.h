#include <string>
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

static FATFS fs;
static FIL fil;
static BYTE work_buffer[FF_MAX_SS];
static FRESULT res; 
static int exitline = 0;

FRESULT mountfs();
FRESULT umountfs();
bool disk_init(); 
FRESULT disk_format();
bool createconfig(const std::string& filename, const std::string& content);
std::string readfilestr(const std::string& filename);