cmake_minimum_required(VERSION 3.10)
project(valtheradb C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2 -fPIC")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JANSSON REQUIRED jansson)

include_directories(${JANSSON_INCLUDE_DIRS} src)

file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "src/*.h")

add_library(valtheradb SHARED ${SOURCES} ${HEADERS})

target_link_libraries(valtheradb ${JANSSON_LIBRARIES})
