cmake_minimum_required(VERSION 3.3)
project(pgn-extract)

set(CMAKE_C_FLAGS_RELEASE "-std=c11 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fstrict-aliasing -flto") #-fno-fat-lto-objects
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer -march=native")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -funroll-loops ")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O1 -s -lgomp")
#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -floop-interchange -ftree-loop-distribution -floop-strip-mine -floop-block -ftree-vectorize") #graphite #TODO: tests

set(CMAKE_C_FLAGS_DEBUG "-g -pg -DDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-lgomp")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -w")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wshadow -Wstrict-aliasing")

file(GLOB_RECURSE SOURCE_FILES "*.c")
file(GLOB_RECURSE HEADER_FILES "*.h")

add_library(pgn-extract STATIC ${SOURCE_FILES} ${HEADER_FILES})