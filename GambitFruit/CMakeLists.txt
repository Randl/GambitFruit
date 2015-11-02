cmake_minimum_required(VERSION 3.3)
project(GambitFruit)

set(CMAKE_CXX_FLAGS_RELEASE "-fno-exceptions -fno-rtti -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fstrict-aliasing -flto") #-fno-fat-lto-objects
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -funroll-loops")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O1 -s")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -floop-interchange -ftree-loop-distribution -floop-strip-mine -floop-block -ftree-vectorize") #graphite

set(CMAKE_CXX_FLAGS_DEBUG "-g -pg -std=c++14 -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wshadow -Wstrict-aliasing -Weffc++")

file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HEADER_FILES "*.h")

add_executable(GambitFruit ${SOURCE_FILES} ${HEADER_FILES})
add_library(GambitFruitStatic STATIC ${SOURCE_FILES} ${HEADER_FILES})