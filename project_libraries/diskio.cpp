#include "ff.h"
#include "diskio.h"
#include "hardware/flash.h"
#include "pico/stdlib.h"
#include <cstring>
#include <string.h> // per memcpy
#include "pico/sync.h"

#define FLASH_FS_OFFSET (0x1E0000)
#define FLASH_SECTOR_SIZE 512
#define FLASH_FS_BASE (XIP_BASE + FLASH_FS_OFFSET)
#define FATFS_FLASH_OFFSET 0x1E0000  

static uint8_t sector_buf[FLASH_SECTOR_SIZE];

DSTATUS disk_initialize(BYTE pdrv) {
    return RES_OK; // Always fine
}

DSTATUS disk_status(BYTE pdrv) {
    return RES_OK; //
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != 0) return RES_PARERR;
    const uint8_t* flash_base = (const uint8_t*)(XIP_BASE + FATFS_FLASH_OFFSET + sector * FLASH_SECTOR_SIZE);
    memcpy(buff, flash_base, count * FLASH_SECTOR_SIZE);
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    return RES_OK; // Ignore the operation
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    return RES_OK; // Ignore the operation
}
