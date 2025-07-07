cmake_minimum_required(VERSION 3.12)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Setting up environment for MacOS...")
    set(USERHOME $ENV{HOME})
    set(sdkVersion 2.1.1)
    set(toolchainVersion 14_2_Rel1)
    set(picotoolVersion 2.1.1)
    set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
    if (EXISTS ${picoVscode})
        include(${picoVscode})
    endif()
endif()

# Check if the OS environment variable PICO_DEV is set, otherwise warn (and quit)
# Inside this directory you should have the PICO SDK, Pimoroni SDK, etc.
if (NOT DEFINED ENV{PICO_DEV})
    message(FATAL_ERROR "Environment variable PICO_DEV is not set")
else()
    set(PICO_DEV $ENV{PICO_DEV})
    file(TO_CMAKE_PATH "${PICO_DEV}" PICO_DEV)
    message(STATUS "PICO_DEV is ${PICO_DEV}")
endif()

set(PICO_BOARD pico_w) # state the pico version here for WiFi to work

# Enable lwIP and Wi-Fi support
set(PICO_CYW43_SUPPORTED 1)
set(PICO_LWIP 1)
set(PICO_LWIP_THREADS 1)  # Enable thread-safe background network

# Enable TinyUSB
set(PICO_TINYUSB_ENABLE 1)

# Load Default Wi-Fi values (SSID/PASSWORD) from wifi_credentials.txt
set(WIFI_CREDENTIALS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/wifi_credentials.txt")
if(EXISTS ${WIFI_CREDENTIALS_FILE})
    file(READ ${WIFI_CREDENTIALS_FILE} FILE_CONTENT)
    string(REGEX REPLACE "\n" ";" CREDENTIALS_LIST "${FILE_CONTENT}")
    list(GET CREDENTIALS_LIST 0 WIFI_SSID)
    list(GET CREDENTIALS_LIST 1 WIFI_PASSWORD)
    message(STATUS "WIFI_SSID: ${WIFI_SSID}")
    message(STATUS "WIFI_PASSWORD: ${WIFI_PASSWORD}")
    add_definitions(-DWIFI_SSID="${WIFI_SSID}")
    add_definitions(-DWIFI_PASSWORD="${WIFI_PASSWORD}")
else()
    message(STATUS "WARNING: WIFI CREDENTIALS NOT DEFINED")
endif()

# Change your executable name to something creative!
set(NAME pico-project) # <-- Name your project/executable here!

# Get sure both SDKs (raspberry and pimoroni) are installed and usable
include(pico_sdk.cmake)
include(pimoroni_pico_sdk.cmake)

# Other places to find headers we are going to need
include_directories(${CMAKE_CURRENT_SOURCE_DIR}) # for lwipopts.h 
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/project_libraries) # for separate classes

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Shrink the usable size for programs in 128 KB 
# so we can safely place a file system
set(PICO_FLASH_SIZE 0x1E0000)

# Initialize the Pico SDK
pico_sdk_init()

# Add your source files
add_executable(${NAME}
    main.cpp # <-- Add source files here!
    project_libraries/color.cpp
    project_libraries/myscreen.cpp
    project_libraries/myled.cpp
    project_libraries/bootsel.cpp
    project_libraries/buttonmgr.cpp
    project_libraries/msc_disk.cpp
    project_libraries/diskio.cpp
    project_libraries/filesystem.cpp
    project_libraries/usb_descriptors.cpp
)

# Adding some libraries here would save you from having to use subdirectories
# in your program #INCLUDES i.e. "/drivers/st7789/st7789.hpp" vs just "st7789.hpp" 
# it's for "complex" libraries that have external dependencies
add_subdirectory(${PIMORONI_PICO_PATH}/drivers/st7789 st7789)
add_subdirectory(${PIMORONI_PICO_PATH}/libraries/pico_graphics pico_graphics)
add_subdirectory(${PIMORONI_PICO_PATH}/libraries/pico_display_2 pico_display_2)
add_subdirectory(${PIMORONI_PICO_PATH}/drivers/fatfs fatfs)

# Include required libraries
include(common/pimoroni_bus)
include(drivers/rgbled/rgbled)
include(drivers/button/button)

# Don't forget to link the libraries you need!
# <-- List libraries down here!
target_link_libraries(${NAME}
#    pico_cyw43_arch_none
    pico_cyw43_arch_lwip_threadsafe_background
    pico_stdlib
    hardware_clocks
    hardware_pio
    st7789
    pico_display_2
    pico_graphics
    rgbled
    button
    pico_lwip_sntp
    hardware_flash
    hardware_adc
    tinyusb_device
    tinyusb_board
    fatfs
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)

# When TRUE it will embed a FAT image at the end of our project
set(INCLUDE_FAT_IMAGE FALSE)  

if(INCLUDE_FAT_IMAGE)
    message(STATUS "APPENDING DISK IMAGE: TRUE") 
    # Looks for a Python Interpreter
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    # Adds a disk image at the end of the compiled code in the .bin file
    set(DISK_IMAGE fat12_disk_128.img)
    set(SOURCE_BIN ${NAME}.bin)
    set(TARGET_BIN ${NAME}+disk.bin)

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/${TARGET_BIN}
        COMMAND ${CMAKE_COMMAND} -E echo "Embedding disk image into ${NAME}.bin file..."
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/disk_images/add-disk.py 
                ${CMAKE_BINARY_DIR}/${SOURCE_BIN}
                ${DISK_IMAGE}
                ${CMAKE_BINARY_DIR}/${TARGET_BIN}
        DEPENDS ${CMAKE_BINARY_DIR}/${SOURCE_BIN} 
                ${CMAKE_SOURCE_DIR}/disk_images/${DISK_IMAGE}
    )

    # Looks for Picotool
    set(PICOTOOL_PATH ${CMAKE_SOURCE_DIR})
    set(FINAL_UF2 ${NAME}_FAT.uf2)

    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/${FINAL_UF2}
        COMMAND ${CMAKE_COMMAND} -E echo "Generating ${TARGET_BIN}.uf2 file..."
        COMMAND ${PICOTOOL_PATH}/picotool uf2 convert 
                ${CMAKE_BINARY_DIR}/${TARGET_BIN} 
                ${CMAKE_SOURCE_DIR}/${FINAL_UF2}
        DEPENDS ${CMAKE_BINARY_DIR}/${TARGET_BIN}
    )
    # Target final que genera el .uf2 i el copia al directori del projecte
    add_custom_target(build_fat ALL
        DEPENDS ${CMAKE_SOURCE_DIR}/${FINAL_UF2}
    )
else()
    message(STATUS "APPENDING DISK IMAGE: FALSE") 
    # Copy .uf2 file to project directory after building
    add_custom_target(copy_uf2 ALL
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
            ${CMAKE_SOURCE_DIR}/${NAME}.uf2
        COMMENT "${NAME}.uf2 copied to project directory"
        DEPENDS ${NAME}
)
endif()

