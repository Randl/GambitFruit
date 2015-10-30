// search.h

#ifndef SEARCH_H
#define SEARCH_H

// includes

#include <csetjmp>

#include "board.h"
#include "list.h"
#include "move.h"
#include "util.h"

// constants

constexpr U8 MultiPVMax = 10;

constexpr U16 DepthMax = 64;
constexpr U16 HeightMax = 256;

constexpr U8 SearchNormal = 0;
constexpr U8 SearchShort = 1;

constexpr U8 SearchUnknown = 0;
constexpr U8 SearchUpper = 1;
constexpr U8 SearchLower = 2;
constexpr U8 SearchExact = 3;

// types

struct search_multipv_t {
    double time;
    S64 node_nb;
    char pv_string[512];
    S32 mate;
    S32 value;
    U16 depth; // TODO: S16?
    U16 max_depth; // S16?
};

struct search_param_t {
    U16 move;
    S32 best_move; // U16
    S32 threat_move;  // U16
    bool reduced;
};

struct search_input_t {
    double time_limit_1;
    double time_limit_2;
    board_t board[1];
    list_t list[1];
    U16 depth_limit; 
    U8 multipv;
    bool time_is_limited;
    bool infinite;
    bool depth_is_limited;
};

struct search_info_t {
    double last_time;
    jmp_buf buf;
    S32 check_nb;
    S32 check_inc;
    bool can_stop;
    bool stop;

};

struct search_root_t {
    list_t list[1];
    S32 depth;
    S32 move_pos;
    S32 move_nb;
    S32 last_value;
    U16 move;
    bool bad_1;
    bool bad_2;
    bool change;
    bool easy;
    bool flag;
};

struct search_best_t {
    mv_t pv[HeightMax];
    S32 value;
    S32 flags;
    S32 depth;
    U16 move;
};

struct search_current_t {
    double time;
    double speed;
    double cpu;
    S64 node_nb;
    board_t board[1];
    my_timer_t timer[1];
    S32 max_depth;
    S32 max_extensions; // Thomas
    S32 multipv;
};

// variables

extern search_param_t SearchStack[HeightMax]; // Thomas
extern search_input_t SearchInput[1];
extern search_info_t SearchInfo[1];
extern search_best_t SearchBest[MultiPVMax];
extern search_root_t SearchRoot[1];
extern search_current_t SearchCurrent[1];

// functions

extern bool depth_is_ok(S32 depth);
extern bool height_is_ok(S32 height);

extern void search_clear();
extern void search();

extern void search_update_best();
extern void search_update_root();
extern void search_update_current();

extern void search_check();

#endif // !defined SEARCH_H

// end of search.h
