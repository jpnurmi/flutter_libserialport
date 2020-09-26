cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "serialport")
project(${PROJECT_NAME} LANGUAGES C)

set(SOURCE_PATH "../../third_party/libserialport")

add_library(${PROJECT_NAME} SHARED
  "${SOURCE_PATH}/linux.c"
  "${SOURCE_PATH}/linux_termios.c"
  "${SOURCE_PATH}/serialport.c"
  "${SOURCE_PATH}/timing.c"
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -std=c99 -Wall -Wextra -pedantic -Wmissing-prototypes -Wshadow)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBSERIALPORT_ATBUILD)
target_include_directories(${PROJECT_NAME} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_PATH}")
