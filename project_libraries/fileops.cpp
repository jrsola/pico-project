#include "fileops.h"

int write2file(const std::string& filename, const std::string& value){
    pico_mount(false);
    int file = pico_open(filename.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (file <0 ){
        return -1;
    }
    int bytes_written = pico_write(file, value.c_str(), value.size());
    pico_close(file);
    if (bytes_written <0 || static_cast<size_t>(bytes_written) != value.size()) {
        return -1;
    }
    pico_unmount();
    return 0;
}

std::string readfile(const std::string& filename){
    pico_mount(false);
    int file = pico_open(filename.c_str(), LFS_O_RDONLY);
    if (file < 0){
        return "";
    }
    const size_t buffer_size = 256;
    char buffer[buffer_size];
    int bytes_read = pico_read(file, buffer, buffer_size - 1);
    pico_close(file);
    pico_unmount();

    if (bytes_read < 0) {
        return "";
    }

    buffer[bytes_read] = '\0';  // Ensure null termination

    return std::string(buffer);
}
