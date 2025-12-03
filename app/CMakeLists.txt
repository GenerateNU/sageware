cmake_minimum_required(VERSION 3.22)

set(ZEPHYR_TOOLCHAIN_VARIANT zephyr)
set(CMAKE_OBJCOPY $ENV{HOME}/zephyr-sdk-0.17.2/xtensa-espressif_esp32_zephyr-elf/bin/xtensa-espressif_esp32_zephyr-elf-objcopy)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

# this didn't work when this was called app. It must be called app because... TO-DO...
<<<<<<< HEAD
target_sources(app PRIVATE src/main.c)
=======
target_sources(app PRIVATE
  src/main.c
  src/motor.c
  src/screen.c
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
>>>>>>> 796d1c7 (Blue screen shows kinda)
