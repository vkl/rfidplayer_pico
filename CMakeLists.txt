# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/vkl/pico/pico-sdk")

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(rfidplayer C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(rfidplayer
  main.c
  # tcp_common.c
  tls_client.c
  cast_message.c
  cast_controllers.c
  rfid_card.c
  player.c
  casts.c
  cjson/cJSON.c
  proto/protobuf-c/protobuf-c.c
  proto/cast_channel.pb-c.c
  )

# pico_generate_pio_header(rfidplayer ${CMAKE_CURRENT_LIST_DIR}/hello.pio)
pico_generate_pio_header(rfidplayer ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)

pico_set_program_name(rfidplayer "rfidplayer")
pico_set_program_version(rfidplayer "0.1")

pico_enable_stdio_uart(rfidplayer 1)
pico_enable_stdio_usb(rfidplayer 0)

# Add the standard include files to the build
target_include_directories(rfidplayer PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
  ${CMAKE_CURRENT_LIST_DIR}/proto/
  ${CMAKE_CURRENT_LIST_DIR}/proto/protobuf-c/
)

execute_process(
    COMMAND date +%s  # Outputs the current Unix timestamp
    OUTPUT_VARIABLE CURRENT_UNIX_TIMESTAMP
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

target_compile_definitions(rfidplayer PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    DEBUG_PRINT="1"
    CURRENT_UNIX_TIMESTAMP=${CURRENT_UNIX_TIMESTAMP}
    PICO_DEBUG_MALLOC=0
)

# link_directories(
#   ${CMAKE_CURRENT_LIST_DIR}/proto/lib
#   )

# Add any user requested libraries
target_link_libraries(rfidplayer 
        hardware_pio
        # hardware_timer
        # hardware_rtc
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        pico_lwip_mbedtls
        pico_mbedtls
        pico_malloc
        # ${CMAKE_CURRENT_LIST_DIR}/proto/lib/libprotobuf-c.a
        # ${CMAKE_CURRENT_LIST_DIR}/proto/lib/libprotobuf-c.la
        )

pico_add_extra_outputs(rfidplayer)

install(CODE 
  "execute_process(COMMAND openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c \"adapter speed 5000\" -c \"program rfidplayer.elf verify reset exit\" RESULT_VARIABLE result)")
