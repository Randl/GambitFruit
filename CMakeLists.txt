cmake_minimum_required(VERSION 2.8.4)
project(src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "C:/Users/ASPIRE/Documents/Projects/GambFruit/bin")

set(SOURCE_FILES
    attack.cpp
    attack.h
    board.cpp
    board.h
    book.cpp
    book.h
    colour.h
    eval.cpp
    eval.h
    fen.cpp
    fen.h
    hash.cpp
    hash.h
    list.cpp
    list.h
    main.cpp
    material.cpp
    material.h
    move.cpp
    move.h
    move_check.cpp
    move_check.h
    move_do.cpp
    move_do.h
    move_evasion.cpp
    move_evasion.h
    move_gen.cpp
    move_gen.h
    move_legal.cpp
    move_legal.h
    option.cpp
    option.h
    pawn.cpp
    pawn.h
    piece.cpp
    piece.h
    posix.cpp
    posix.h
    protocol.cpp
    protocol.h
    pst.cpp
    pst.h
    pv.cpp
    pv.h
    random.cpp
    random.h
    search.cpp
    search.h
    search_full.cpp
    search_full.h
    see.cpp
    see.h
    sort.cpp
    sort.h
    square.cpp
    square.h
    trans.cpp
    trans.h
    util.cpp
    util.h
    value.cpp
    value.h
    vector.cpp
    vector.h)

add_executable(src ${SOURCE_FILES})