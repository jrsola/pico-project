cmake_minimum_required(VERSION 3.12)

# Change your executable name to something creative!
set(NAME pico-project) # <-- Name your project/executable here!

set(PICO_BOARD pico_w) # state the pico version here for WiFi to work

# Get sure both SDKs (raspberry and pimoroni) are installed and usable
include(pico_sdk_template.cmake)
include(pimoroni_pico_template.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# Adding some libraries here would save you from having to use subdirectories
# in your program #INCLUDES i.e. "/drivers/st7789/st7789.hpp" vs just "st7789.hpp" 
# it's for "complex" libraries that have external dependencies
add_subdirectory($ENV{PIMORONI_PICO_PATH}/drivers/st7789 st7789)
add_subdirectory($ENV{PIMORONI_PICO_PATH}/libraries/pico_graphics pico_graphics)
add_subdirectory($ENV{PIMORONI_PICO_PATH}/libraries/pico_display_2 pico_display_2)

# Add your source files
add_executable(${NAME}
    main.cpp # <-- Add source files here!
)

# Include required libraries
include(common/pimoroni_bus)
include(drivers/rgbled/rgbled)
include(drivers/button/button)

# Don't forget to link the libraries you need!
# <-- List libraries down here!
target_link_libraries(${NAME}
    pico_stdlib
    pico_cyw43_arch_none
    hardware_clocks
    hardware_pio
    st7789
    pico_display_2
    pico_graphics
    rgbled
    button
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
