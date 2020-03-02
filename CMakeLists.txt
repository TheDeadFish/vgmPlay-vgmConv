project(vgmConv)
cmake_minimum_required(VERSION 3.16)

set( EXECUTABLE_OUTPUT_PATH ../bin)
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../bin)
set( CMAKE_STATIC_LIBRARY_PREFIX "")
add_link_options(-s -Wl,--gc-sections)

include_directories(lib .)
file(GLOB LIB_SRC lib/*.cpp src/*.cpp src/vgx/*.cpp src/codec/*.cpp)
file(GLOB APP_SRC src/app/*.cpp)

add_definitions(-DHAS_ZLIB)
add_library(libVgmConv ${LIB_SRC})
add_executable(vgmConv ${APP_SRC})
target_link_libraries(vgmConv libVgmConv -lz)
