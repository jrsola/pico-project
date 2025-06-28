#include "fes_flash.h"

int exitline = 0;

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

FRESULT mountfs(){
    return f_mount(&fs, "/", 1);
}

FRESULT umountfs(){
    return f_mount(NULL, "/", 0);
}

FRESULT disk_format() {
    MKFS_PARM mkfs_param = {
        .fmt = FM_FAT,
        .n_fat = 1,
        .align = 0,
        .n_root = 0,
        .au_size = 0
    };
    return f_mkfs("", &mkfs_param, work_buffer, sizeof(work_buffer));
}

bool disk_init() {
    if (mountfs() != FR_OK) {
        if (disk_format() != FR_OK) return false;
        if (mountfs() != FR_OK) return false;
    }
    res = umountfs(); // umount
    return res == FR_OK;
}

// Creates a file with a string as a content, overwrites previous file
bool createconfig(const std::string& filename, const std::string& content){

    if ((res = mountfs()) != FR_OK) {
        exitline = 1000;
        return res == FR_OK;
    }

    res = f_open(&fil, filename.c_str(), FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        exitline = 1001;
        return res == FR_OK;
    }


    UINT bytes_written = 0;
    char buffer[512];
    int size = snprintf(buffer, sizeof(buffer), "tuputamadre");

    res = f_write(&fil, buffer, size, &bytes_written);
    if (res != FR_OK || bytes_written != content.size()) {
        exitline = bytes_written;
        f_close(&fil);
        return false;
    }
    res = f_sync(&fil);
    if (res != FR_OK) {
        exitline = 1003;
        f_close(&fil);
        return false;
    }
    exitline = content.size();
    f_close(&fil);
    umountfs();
    return true;
}

// Reads a file and returns the string it contains
std::string readfilestr(const std::string& filename) {
    char buffer[128];
    UINT bytes_read;
    std::string content;

    // Pas 1: Mount del sistema de fitxers
    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        exitline = 1; return "ERROR";
    }

    // Pas 2: Obrim el fitxer
    res = f_open(&fil, filename.c_str(), FA_READ);
    if (res != FR_OK) {
        exitline = 2; return "ERROR";
    }

    if (f_size(&fil) == 0) {
     exitline = 99; return "ERROR";
}


    // Pas 3: Llegim el contingut en blocs
    do {
        res = f_read(&fil, buffer, sizeof(buffer) - 1, &bytes_read);
        if (res != FR_OK) {
            exitline = 3; break;
        }
        buffer[bytes_read] = '\0';
        content += buffer;
    } while (bytes_read > 0);

    // Pas 4: Tanquem el fitxer
    f_close(&fil);

    if (content.empty()) {
        exitline = 4;
        return "ERROR";
    }

    return content;
}