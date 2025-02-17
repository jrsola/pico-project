# This file can be dropped into a project to help locate the Pimoroni Pico libraries
# It will also set up the required include and module search paths.

# Check if the OS environment variable PIMORONI_PICO_PATH is set, otherwise warn (and quit)
if (NOT DEFINED ENV{PIMORONI_PICO_PATH})
    message(FATAL_ERROR "OS environment variable PIMORONI_PICO_PATH does not seem to be defined")
endif()

set(PIMORONI_PICO_PATH $ENV{PIMORONI_PICO_PATH})
get_filename_component(PIMORONI_PICO_PATH "${PIMORONI_PICO_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

if (NOT EXISTS ${PIMORONI_PICO_PATH})
    message(FATAL_ERROR "Directory '${PIMORONI_PICO_PATH}' not found")
endif()

if (NOT EXISTS ${PIMORONI_PICO_PATH}/pimoroni_pico_import.cmake)
    message(FATAL_ERROR "Directory '${PIMORONI_PICO_PATH}' does not appear to contain the Pimoroni Pico libraries")
endif()

message("PIMORONI_PICO_PATH is ${PIMORONI_PICO_PATH}")
set(PIMORONI_PICO_PATH ${PIMORONI_PICO_PATH} CACHE PATH "Path to the Pimoroni Pico libraries" FORCE)

include_directories(${PIMORONI_PICO_PATH})
list(APPEND CMAKE_MODULE_PATH ${PIMORONI_PICO_PATH})

