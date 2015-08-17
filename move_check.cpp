
// move_check.cpp

// includes

#include "attack.h"
#include "colour.h"
#include "fen.h"
#include "list.h"
#include "move.h"
#include "move_check.h"
#include "move_do.h"
#include "move_gen.h"
#include "piece.h"
#include "square.h"
#include "util.h"

// prototypes

static void add_quiet_checks      (list_t * list, const board_t * board);

static void add_castle_checks     (list_t * list, board_t * board);

static void add_check             (list_t * list, int_fast32_t move, board_t * board);

static void find_pins             (uint_fast16_t list[], const board_t * board);

// functions

// gen_quiet_checks()

void gen_quiet_checks(list_t * list, board_t * board)  {

    ASSERT(list!=nullptr);
    ASSERT(board!=nullptr);
    ASSERT(!board_is_check(board));

    LIST_CLEAR(list);

    add_quiet_checks(list,board);
    add_castle_checks(list,board);

    // debug
    ASSERT(list_is_ok(list));
}

// add_quiet_checks()

static void add_quiet_checks(list_t * list, const board_t * board)  {
    ASSERT(list!=nullptr);
    ASSERT(board!=nullptr);

    // init
    const int_fast32_t me = board->turn, opp = COLOUR_OPP(me), king = KING_POS(board,opp);

    uint_fast16_t pin[8+1];
    find_pins(pin,board);

    // indirect checks
    for (const sq_t *ptr = pin; *ptr != SquareNone; ++ptr)  {

        const int_fast32_t piece = board->square[*ptr];

        ASSERT(is_pinned(board,*ptr,opp));

        if (PIECE_IS_PAWN(piece))  {

            const int_fast32_t inc = PAWN_MOVE_INC(me), rank = PAWN_RANK(*ptr,me);

            if (rank != Rank7)  { // promotes are generated with captures
                int_fast32_t to = *ptr + inc;
                if (board->square[to] == Empty)  {
                    if (DELTA_INC_LINE(to-king) != DELTA_INC_LINE(*ptr-king))  {
                        ASSERT(!SQUARE_IS_PROMOTE(to));
                        LIST_ADD(list,MOVE_MAKE(*ptr,to));
                        if (rank == Rank2)  {
                            to = *ptr + (2*inc);
                            if (board->square[to] == Empty)  {
                                ASSERT(DELTA_INC_LINE(to-king)!=DELTA_INC_LINE(*ptr-king));
                                ASSERT(!SQUARE_IS_PROMOTE(to));
                                LIST_ADD(list,MOVE_MAKE(*ptr,to));
                            }
                        }
                    }
                }
            }
        } else if (PIECE_IS_SLIDER(piece))  {
            for (const inc_t *inc_ptr = PIECE_INC(piece); *inc_ptr != IncNone; ++inc_ptr)  {
                for (int_fast32_t to = *ptr+*inc_ptr; board->square[to] == Empty; to += *inc_ptr)  {
                    ASSERT(DELTA_INC_LINE(to-king)!=DELTA_INC_LINE(*ptr-king));
                    LIST_ADD(list,MOVE_MAKE(*ptr,to));
                }
            }
        } else {
            for (const inc_t *inc_ptr = PIECE_INC(piece); *inc_ptr != IncNone; ++inc_ptr)  {
                int_fast32_t to = *ptr + *inc_ptr;
                if (board->square[to] == Empty)  {
                    if (DELTA_INC_LINE(to-king) != DELTA_INC_LINE(*ptr-king))  {
                        LIST_ADD(list,MOVE_MAKE(*ptr,to));
                    }
                }
            }
        }
    }

    // piece direct checks
    for (auto ptr = board->piece[me].begin() + 1; ptr != board->piece[me].end(); ++ptr)  { // HACK: no king

        const inc_t *inc_ptr;
        int_fast32_t piece;
        for (const sq_t *ptr_2 = pin; *ptr_2 != SquareNone; ++ptr_2)  {
            if (*ptr_2 == *ptr) goto next_piece;
        }

        ASSERT(!is_pinned(board,*ptr,opp));

        piece = board->square[*ptr];
        inc_ptr = PIECE_INC(piece);

        if (PIECE_IS_SLIDER(piece))  {
            for (; *inc_ptr != IncNone; ++inc_ptr)  {
                for (int_fast32_t to = *ptr+*inc_ptr; board->square[to] == Empty; to += *inc_ptr)  {
                    if (PIECE_ATTACK(board,piece,to,king))  {
                        LIST_ADD(list,MOVE_MAKE(*ptr,to));
                    }
                }
            }

        } else {
            for (; *inc_ptr != IncNone; ++inc_ptr)  {
                int_fast32_t to = *ptr + *inc_ptr;
                if (board->square[to] == Empty)  {
                    if (PSEUDO_ATTACK(piece,king-to))  {
                        LIST_ADD(list,MOVE_MAKE(*ptr,to));
                    }
                }
            }
        }

next_piece: ;//TODO: refactor bool + continue
    }

    // pawn direct checks
    const int_fast32_t inc = PAWN_MOVE_INC(me), pawn = PAWN_MAKE(me);
    int_fast32_t to = king - (inc-1);
    ASSERT(PSEUDO_ATTACK(pawn,king-to));

    int_fast32_t from = to - inc;
    if (board->square[from] == pawn)  {
        if (board->square[to] == Empty)  {
            ASSERT(!SQUARE_IS_PROMOTE(to));
            LIST_ADD(list,MOVE_MAKE(from,to));
        }
    } else {
        from = to - (2*inc);
        if (board->square[from] == pawn)  {
            if (PAWN_RANK(from,me) == Rank2
             && board->square[to] == Empty
             && board->square[from+inc] == Empty)  {
                ASSERT(!SQUARE_IS_PROMOTE(to));
                LIST_ADD(list,MOVE_MAKE(from,to));
            }
        }
    }

    to = king - (inc+1);
    ASSERT(PSEUDO_ATTACK(pawn,king-to));

    from = to - inc;
    if (board->square[from] == pawn)  {
        if (board->square[to] == Empty)  {
            ASSERT(!SQUARE_IS_PROMOTE(to));
            LIST_ADD(list,MOVE_MAKE(from,to));
        }
    } else {
        from = to - (2*inc);
        if (board->square[from] == pawn)  {
            if (PAWN_RANK(from,me) == Rank2
             && board->square[to] == Empty
             && board->square[from+inc] == Empty)  {
                ASSERT(!SQUARE_IS_PROMOTE(to));
                LIST_ADD(list,MOVE_MAKE(from,to));
            }
        }
    }
}

// add_castle_checks()

static void add_castle_checks(list_t * list, board_t * board)  {

    ASSERT(list!=nullptr);
    ASSERT(board!=nullptr);
    ASSERT(!board_is_check(board));

    if (COLOUR_IS_WHITE(board->turn))  {

        if ((board->flags & FlagsWhiteKingCastle) != 0
          && board->square[F1] == Empty
          && board->square[G1] == Empty
          && !is_attacked(board,F1,Black))  {
            add_check(list,MOVE_MAKE_FLAGS(E1,G1,MoveCastle),board);
        }

        if ((board->flags & FlagsWhiteQueenCastle) != 0
          && board->square[D1] == Empty
          && board->square[C1] == Empty
          && board->square[B1] == Empty
          && !is_attacked(board,D1,Black))  {
            add_check(list,MOVE_MAKE_FLAGS(E1,C1,MoveCastle),board);
        }

    } else { // black

        if ((board->flags & FlagsBlackKingCastle) != 0
          && board->square[F8] == Empty
          && board->square[G8] == Empty
          && !is_attacked(board,F8,White))  {
            add_check(list,MOVE_MAKE_FLAGS(E8,G8,MoveCastle),board);
        }

        if ((board->flags & FlagsBlackQueenCastle) != 0
          && board->square[D8] == Empty
          && board->square[C8] == Empty
          && board->square[B8] == Empty
          && !is_attacked(board,D8,White))  {
            add_check(list,MOVE_MAKE_FLAGS(E8,C8,MoveCastle),board);
        }
    }
}

// add_check()

static void add_check(list_t * list, int_fast32_t move, board_t * board)  {

    ASSERT(list!=nullptr);
    ASSERT(move_is_ok(move));
    ASSERT(board!=nullptr);

    undo_t undo[1];
    move_do(board,move,undo);
    if (IS_IN_CHECK(board,board->turn)) LIST_ADD(list,move);
    move_undo(board,move,undo);
}

// move_is_check()

bool move_is_check(int_fast32_t move, board_t * board)  {

    ASSERT(move_is_ok(move));
    ASSERT(board!=nullptr);

    // slow test for complex moves
    if (MOVE_IS_SPECIAL(move))  {

        undo_t undo[1];
        move_do(board,move,undo);
        bool check = IS_IN_CHECK(board,board->turn);
        move_undo(board,move,undo);

        return check;
    }

   // init
    const int_fast32_t me = board->turn, opp = COLOUR_OPP(me), king = KING_POS(board,opp);
    const int_fast32_t from = MOVE_FROM(move), to = MOVE_TO(move), piece = board->square[from];
    ASSERT(COLOUR_IS(piece,me));

    // direct check
    if (PIECE_ATTACK(board,piece,to,king)) return true;

    // indirect check
    if (is_pinned(board,from,opp)
     && DELTA_INC_LINE(king-to) != DELTA_INC_LINE(king-from))  {
        return true;
    }

    return false;
}

// find_pins()

static void find_pins(uint_fast16_t list[], const board_t * board)  {

    ASSERT(list!=nullptr);
    ASSERT(board!=nullptr);

    // init
    const int_fast32_t me = board->turn, opp = COLOUR_OPP(me), king = KING_POS(board,opp);

    for (auto ptr = board->piece[me].begin() + 1; ptr != board->piece[me].end(); ++ptr)  { // HACK: no king

        const int_fast32_t piece = board->square[*ptr], delta = king - *ptr;
        ASSERT(delta_is_ok(delta));

        if (PSEUDO_ATTACK(piece,delta))  {

            ASSERT(PIECE_IS_SLIDER(piece));

            const int_fast32_t inc = DELTA_INC_LINE(delta);
            ASSERT(inc!=IncNone);

            ASSERT(SLIDER_ATTACK(piece,inc));

            int_fast32_t sq = *ptr, capture;
            do sq += inc; while ((capture=board->square[sq]) == Empty);

            ASSERT(sq!=king);

            if (COLOUR_IS(capture,me))  {
                const int_fast32_t pin = sq;
                do sq += inc; while (board->square[sq] == Empty);
                if (sq == king) *list++ = pin;
            }
        }
    }

    *list = SquareNone;
}

// end of move_check.cpp

