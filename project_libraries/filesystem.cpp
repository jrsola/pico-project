#include "filesystem.h"

int exitline = 0;

bool mount_filesystem() {
    FRESULT res = f_mount(&fs, "", 1);
    return (res == FR_OK);
}

bool create_text_file(const char* filename, const char* contents) {
    FRESULT res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) return false;

    UINT written;
    res = f_write(&file, contents, strlen(contents), &written);
    f_close(&file);

    return res == FR_OK && written == strlen(contents);
}

bool read_text_file(const char* filename, char* buffer, size_t max_size) {
    FRESULT res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) return false;

    UINT read_bytes = 0;
    res = f_read(&file, buffer, max_size - 1, &read_bytes);
    f_close(&file);

    if (res != FR_OK) return false;

    buffer[read_bytes] = '\0'; // assegura final de cadena
    return true;
}

std::string read_file(const std::string& path) {
    FIL fil;
    if (f_open(&fil, path.c_str(), FA_READ) != FR_OK)
        return "ERROR";

    char buffer[128];
    UINT bytesRead;
    f_read(&fil, buffer, sizeof(buffer) - 1, &bytesRead);
    buffer[bytesRead] = '\0';
    f_close(&fil);

    return std::string(buffer);
}