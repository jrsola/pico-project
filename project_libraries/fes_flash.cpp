#include "fes_flash.h"

static FATFS fs;

DSTATUS disk_initialize(BYTE pdrv) {
    return RES_OK;
}

DSTATUS disk_status(BYTE pdrv) {
    return RES_OK;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    memcpy(buff, (const void *)(XIP_BASE + FATFS_OFFSET + sector * FATFS_SECTOR_SIZE), count * FATFS_SECTOR_SIZE);
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    uint32_t ints = save_and_disable_interrupts();

    for (UINT i = 0; i < count; ++i) {
        uint32_t flash_offset = FATFS_OFFSET + (sector + i) * FATFS_SECTOR_SIZE;
        flash_range_erase(flash_offset, FLASH_SECTOR_SIZE);
        flash_range_program(flash_offset, buff + i * FATFS_SECTOR_SIZE, FLASH_SECTOR_SIZE);
    }

    restore_interrupts(ints);
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    switch (cmd) {
        case CTRL_SYNC: return RES_OK;
        case GET_SECTOR_SIZE: *(WORD*)buff = FATFS_SECTOR_SIZE; return RES_OK;
        case GET_BLOCK_SIZE: *(DWORD*)buff = FLASH_SECTOR_SIZE / FATFS_SECTOR_SIZE; return RES_OK;
        case GET_SECTOR_COUNT: *(DWORD*)buff = FATFS_SECTOR_COUNT; return RES_OK;
        default: return RES_PARERR;
    }
}

bool disk_init() {
    FRESULT res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        MKFS_PARM opt = {
            .fmt = FM_FAT,
            .n_fat = 1,
            .align = 0,
            .n_root = 0,
            .au_size = 0
        };
        res = f_mkfs("",&opt,nullptr,0);
        if (res != FR_OK) return false;

        res = f_mount(&fs, "", 1);
        if (res != FR_OK) return false;
    }
    return true;
}