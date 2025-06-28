#include "ff.h"
#include "diskio.h"
#include "hardware/flash.h"
#include "pico/stdlib.h"
#include <cstring>
#include "pico/sync.h"

#define FLASH_FS_OFFSET (PICO_FLASH_SIZE_BYTES - 128 * 1024)
#define FLASH_SECTOR_SIZE 512
#define FLASH_SECTOR_COUNT (128 * 1024 / 512)

static uint8_t sector_buf[FLASH_SECTOR_SIZE];

// Només suportem un dispositiu: Drive 0
DSTATUS disk_initialize(BYTE pdrv) {
    return (pdrv == 0) ? RES_OK : STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv) {
    return (pdrv == 0) ? RES_OK : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != 0 || (sector + count) > FLASH_SECTOR_COUNT)
        return RES_PARERR;

    for (UINT i = 0; i < count; ++i) {
        const uint8_t* src = (const uint8_t*)(XIP_BASE + FLASH_FS_OFFSET + (sector + i) * FLASH_SECTOR_SIZE);
        memcpy(buff + i * FLASH_SECTOR_SIZE, src, FLASH_SECTOR_SIZE);
    }

    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != 0 || (sector + count) > FLASH_SECTOR_COUNT)
        return RES_PARERR;

    for (UINT i = 0; i < count; ++i) {
        uint32_t addr = FLASH_FS_OFFSET + (sector + i) * FLASH_SECTOR_SIZE;

        // Copia del sector a esborrar
        memcpy(sector_buf, (const void*)(XIP_BASE + addr), FLASH_SECTOR_SIZE);
        memcpy(sector_buf, buff + i * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);

        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(addr, FLASH_SECTOR_SIZE);
        flash_range_program(addr, sector_buf, FLASH_SECTOR_SIZE);
        restore_interrupts(ints);
    }

    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    if (pdrv != 0) return RES_PARERR;

    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = FLASH_SECTOR_SIZE;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;  // Un sector per block erase
            return RES_OK;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = FLASH_SECTOR_COUNT;
            return RES_OK;
        default:
            return RES_PARERR;
    }
}
