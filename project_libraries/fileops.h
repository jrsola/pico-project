#include <string>
#include "lfs.h"
#include "pico_hal.h"
#define FS_SIZE (512 * 1024)  // 512KB reserved

std::string readfile(const std::string& filename);
int write2file(const std::string& filename, const std::string& value);