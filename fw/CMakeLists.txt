add_executable(mycropad)

# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(mycropad 
    ${CMAKE_CURRENT_SOURCE_DIR}/ws2812.pio 
    OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)

target_sources(mycropad PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hid_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cdc_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rotary_encoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/leds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/keymap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc.c
)

target_include_directories(mycropad PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

target_link_libraries(mycropad 
    pico_stdlib
    hardware_pio
    hardware_flash
    hardware_sync
    tinyusb_device
    tinyusb_board
)

target_compile_definitions(mycropad PUBLIC PICO_DEFAULT_UART=0)
# target_compile_definitions(mycropad PUBLIC PICO_DEFAULT_UART_TX_PIN=16)
# target_compile_definitions(mycropad PUBLIC PICO_DEFAULT_UART_RX_PIN=17)

pico_enable_stdio_usb(mycropad 0)
pico_enable_stdio_uart(mycropad 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(mycropad)
