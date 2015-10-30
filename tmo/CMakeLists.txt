cmake_minimum_required(VERSION 3.3)
project(tmo)

set(CMAKE_CXX_FLAGS_RELEASE "-fno-exceptions -fno-rtti -std=c++14 -fopenmp -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fstrict-aliasing -flto") #-fno-fat-lto-objects
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -funroll-loops ")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O1 -s -lgomp")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -floop-interchange -ftree-loop-distribution -floop-strip-mine -floop-block -ftree-vectorize") #graphite #TODO: tests

set(CMAKE_CXX_FLAGS_DEBUG "-g -fopenmp -std=c++14 -DDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-lgomp")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -w")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wshadow -Wstrict-aliasing -Weffc++")

file(GLOB_RECURSE SOURCE_FILES "*.cpp")
file(GLOB_RECURSE HEADER_FILES "*.h")

add_definitions("-DTMO_OPT")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../GambitFruit ${CMAKE_CURRENT_BINARY_DIR}/GF)
add_executable(tmo ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(tmo GambitFruitStatic)