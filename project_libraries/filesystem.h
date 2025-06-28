#include <cstring>
#include <cstdio>
#include "ff.h"


extern int exitline; // for debugging
static FATFS fs;
static FIL file;
static constexpr size_t WORKBUFFER_SIZE = 4096;
static uint8_t work_buffer[WORKBUFFER_SIZE];
static FRESULT res;


bool mount_filesystem();
bool format_filesystem();
bool create_text_file(const char* filename, const char* contents);
bool read_text_file(const char* filename, char* buffer, size_t max_size);