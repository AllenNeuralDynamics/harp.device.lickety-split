cmake_minimum_required(VERSION 3.13)

include(lib/pico-sdk/pico_sdk_init.cmake)

project(lickety_split)


set(CMAKE_CXX_STANDARD 17)

# Enable try/catch exception interface.
#set(PICO_CXX_ENABLE_EXCEPTIONS 1)


# Compile for profiling/debugging/etc. Default: none enabled.
add_definitions(-DDEBUG)
#add_definitions(-DPROFILE_CPU)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add Harp Core as separate library.
add_subdirectory(lib/harp.core.rp2040/firmware)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

add_library(core1_lick_detection
    src/core1_lick_detection.cpp
)

add_library(ad9833
    src/ad9833.cpp
)

#add_library(continuous_adc
#    src/continuous_adc.cpp
#)

add_library(lick_detector
    src/lick_detector.cpp
)

add_library(pio_ads70x9
    src/pio_ads70x9.cpp
)

pico_generate_pio_header(pio_ads70x9
    ${CMAKE_CURRENT_LIST_DIR}/src/pio_ads70x9.pio
)

# Specify where to look for header files if they're not all in the same place.
#target_include_directories(${PROJECT_NAME} PUBLIC inc)
# Specify where to look for header files if they're all in one place.
include_directories(inc)

#add_compile_options(-O3)

# Link libraries to the targets that need them.

target_link_libraries(ad9833 pico_stdlib hardware_spi)
target_link_libraries(lick_detector hardware_dma pico_stdlib)
target_link_libraries(pio_ads70x9 pico_stdlib hardware_pio hardware_irq
                      hardware_dma)
target_link_libraries(core1_lick_detection pico_stdlib hardware_irq
                      lick_detector hardware_dma pico_multicore pio_ads70x9
                      hardware_pio)
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_pwm ad9833
                      core1_lick_detection pico_multicore harp_sync harp_c_app)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})

# USB serial. Enable this for every library/executable that uses it.
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_usb(ad9833 1)
pico_enable_stdio_usb(lick_detector 1)
pico_enable_stdio_usb(core1_lick_detection 1)

if(DEBUG)
    message(WARNING "Debug printf() messages enabled from harp core to UART \
            with baud rate 921600.")
    pico_enable_stdio_uart(${PROJECT_NAME} 1)
endif()
