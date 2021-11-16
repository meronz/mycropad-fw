add_executable(mycropad
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rotary_encoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
)

target_include_directories(mycropad PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(mycropad 
    pico_stdlib
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(mycropad 0)
pico_enable_stdio_uart(mycropad 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(mycropad)
