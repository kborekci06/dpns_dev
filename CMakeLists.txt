# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(dpns_dev C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(dpns_dev dpns_dev.c imu.c co2.c hw_config.c)

target_sources(dpns_dev PRIVATE hw_config.c)

pico_set_program_name(dpns_dev "dpns_dev")
pico_set_program_version(dpns_dev "0.1")

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(dpns_dev 0)
pico_enable_stdio_usb(dpns_dev 1)

# Add the standard library to the build
target_link_libraries(dpns_dev
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(dpns_dev PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(dpns_dev 
        hardware_spi
        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico   # FatFS SD card library target
        hardware_i2c
        hardware_uart
        pico_multicore
        )

pico_add_extra_outputs(dpns_dev)

