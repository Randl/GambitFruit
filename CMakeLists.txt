cmake_minimum_required(VERSION 3.3)
project(GambitFruit)

set(CMAKE_CXX_FLAGS_RELEASE "-fno-exceptions -fno-rtti -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fstrict-aliasing -flto") #-fno-fat-lto-objects
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -funroll-loops")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O1 -s")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -floop-interchange -ftree-loop-distribution -floop-strip-mine -floop-block -ftree-vectorize") #graphite

set(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++14 -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wshadow -Wstrict-aliasing -Weffc++")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(HEADERS
        attack.h
        scorpio_egbb.h
        board.h
        book.h
        colour.h
        eval.h
        fen.h
        hash.h
        list.h
        material.h
        move.h
        move_check.h
        move_do.h
        move_evasion.h
        move_gen.h
        move_legal.h
        option.h
        pawn.h
        piece.h
        posix.h
        protocol.h
        pst.h
        pv.h
        random.h
        search.h
        search_full.h
        see.h
        sort.h
        square.h
        trans.h
        util.h
        value.h
        vector.h)

set(SOURCE_FILES
        attack.cpp
        scorpio_egbb.cpp
        board.cpp
        book.cpp
        eval.cpp
        fen.cpp
        hash.cpp
        list.cpp
        main.cpp
        material.cpp
        move.cpp
        move_check.cpp
        move_do.cpp
        move_evasion.cpp
        move_gen.cpp
        move_legal.cpp
        option.cpp
        pawn.cpp
        piece.cpp
        posix.cpp
        protocol.cpp
        pst.cpp
        pv.cpp
        random.cpp
        search.cpp
        search_full.cpp
        see.cpp
        sort.cpp
        square.cpp
        trans.cpp
        util.cpp
        value.cpp
        vector.cpp)

add_executable(GambitFruit ${SOURCE_FILES})