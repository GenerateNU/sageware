cmake_minimum_required(VERSION 3.22)

set(ZEPHYR_TOOLCHAIN_VARIANT zephyr)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

# this didn't work when this was called app. It must be called app because... TO-DO...
target_sources(app PRIVATE
  src/main.c
  src/motor.c
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
