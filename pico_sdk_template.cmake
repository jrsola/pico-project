# This can be dropped into an external project to help locate the Pico SDK
# It should be include()ed prior to project()

# Check if the OS environment variable PICO_SDK_PATH is set, otherwise warn (and quit)
if (NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "OS environment variable PICO_SDK_PATH does not seem to be defined")
endif()

set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
get_filename_component(PICO_SDK_PATH "${PICO_SDK_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

if (NOT EXISTS ${PICO_SDK_PATH})
    message(FATAL_ERROR "Directory '${PICO_SDK_PATH}' not found")
endif()

set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)
if (NOT EXISTS ${PICO_SDK_INIT_CMAKE_FILE})
    message(FATAL_ERROR "Directory '${PICO_SDK_PATH}' does not appear to contain the Raspberry Pi Pico SDK")
endif ()

message("PIMORONI_PICO_PATH is ${PIMORONI_PICO_PATH}")
set(PICO_SDK_PATH ${PICO_SDK_PATH} CACHE PATH "Path to the Raspberry Pi Pico SDK" FORCE)

include(${PICO_SDK_INIT_CMAKE_FILE})

