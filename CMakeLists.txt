project(vgmConv)
cmake_minimum_required(VERSION 3.16)
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# linker flags
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_link_options(-s -Wl,--gc-sections)	
	endif()
endif()

# unicode build flags
if (WIN32)
	add_link_options(-municode)
endif()

# compile flags
add_compile_options(-std=gnu++14)

# warnings
add_compile_options(-Wno-delete-non-virtual-dtor)
add_compile_options(-Wno-invalid-source-encoding)

# source files
include_directories(lib .)
file(GLOB LIB_SRC lib/*.cpp src/*.cpp src/vgx/*.cpp src/codec/*.cpp)
file(GLOB APP_SRC src/app/*.cpp)

add_definitions(-DHAS_ZLIB)
add_library(libVgmConv ${LIB_SRC})
add_executable(vgmConv ${APP_SRC})
target_link_libraries(vgmConv libVgmConv -lz)
