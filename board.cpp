
// board.cpp

// includes

#include "attack.h"
#include "board.h"
#include "colour.h"
#include "fen.h"
#include "hash.h"
#include "list.h"
#include "move.h"
#include "move_do.h"
#include "move_evasion.h"
#include "move_gen.h"
#include "move_legal.h"
#include "pawn.h" // TODO: bit.h
#include "piece.h"
#include "pst.h"
#include "util.h"
#include "value.h"

// constants

static const bool UseSlowDebug = false;

// functions

// board_is_ok()

bool board_is_ok(const board_t * board) {

    if (board == nullptr) return false;

    // optional heavy DEBUG mode
    if (!UseSlowDebug) return true;

    // squares
    for (int_fast32_t sq = 0; sq < SquareNb; sq++) {

        int_fast32_t piece = board->square[sq], pos = board->pos[sq];

        if (SQUARE_IS_OK(sq)) {

            // inside square
            if (piece == Empty) {
                if (pos != -1) return false;
            } else {
                if (!piece_is_ok(piece)) return false;
                if (!PIECE_IS_PAWN(piece)) {

                    const int_fast8_t colour = PIECE_COLOUR(piece);
                    if (pos < 0 || pos >= board->piece[colour].size()) return false;
                    if (board->piece[colour][pos] != sq) return false;
                } else { // pawn
                    if (SQUARE_IS_PROMOTE(sq)) return false;

                    const int_fast8_t colour = PIECE_COLOUR(piece);
                    if (pos < 0 || pos >= board->pawn[colour].size()) return false;
                    if (board->pawn[colour][pos] != sq) return false;
                }
            }

        } else {

        // edge square
        if (piece != Edge) return false;
        if (pos != -1) return false;
        }
    }

    // piece lists
    for (int_fast8_t colour = 0; colour < ColourNb; colour++) {

        // piece list
        int_fast32_t size = board->piece[colour].size();
        if (size < 1 || size > 16) return false;

        for (int_fast32_t pos = 0; pos < size; pos++) {
            int_fast32_t sq = board->piece[colour][pos];
            if (!SQUARE_IS_OK(sq)) return false;

            if (board->pos[sq] != pos) return false;

            int_fast32_t piece = board->square[sq];
            if (!COLOUR_IS(piece,colour)) return false;
            if (pos == 0 && !PIECE_IS_KING(piece)) return false;
            if (pos != 0 && PIECE_IS_KING(piece)) return false;

            if (pos != 0 && PIECE_ORDER(piece) > PIECE_ORDER(board->square[board->piece[colour][pos-1]])) {
                return false;
            }
        }

        int_fast32_t sq = board->piece[colour][size];
        if (sq != SquareNone) return false;

        // pawn list

        size = board->pawn[colour].size();
        if (size < 0 || size > 8) return false;

        for (int_fast32_t pos = 0; pos < size; pos++) {

            int_fast32_t sq = board->pawn[colour][pos];
            if (!SQUARE_IS_OK(sq)) return false;
            if (SQUARE_IS_PROMOTE(sq)) return false;

            if (board->pos[sq] != pos) return false;

            int_fast32_t piece = board->square[sq];
            if (!COLOUR_IS(piece,colour)) return false;
            if (!PIECE_IS_PAWN(piece)) return false;
        }

        sq = board->pawn[colour][size];
        if (sq != SquareNone) return false;

        // piece total
        if (board->piece[colour].size() + board->pawn[colour].size() > 16) return false;
    }

    // material
    if (board->piece_nb != board->piece[White].size() + board->pawn[White].size()
                         + board->piece[Black].size() + board->pawn[Black].size()) {
        return false;
    }

    if (board->number[WhitePawn12] != board->pawn[White].size()) return false;
    if (board->number[BlackPawn12] != board->pawn[Black].size()) return false;
    if (board->number[WhiteKing12] != 1) return false;
    if (board->number[BlackKing12] != 1) return false;

    // misc
    if (!COLOUR_IS_OK(board->turn)) return false;
    if (board->ply_nb < 0) return false;
    if (board->stack.size() < board->ply_nb) return false;
    if (board->cap_sq != SquareNone && !SQUARE_IS_OK(board->cap_sq)) return false;
    if (board->opening != board_opening(board)) return false;
    if (board->endgame != board_endgame(board)) return false;
    if (board->key != hash_key(board)) return false;
    if (board->pawn_key != hash_pawn_key(board)) return false;
    if (board->material_key != hash_material_key(board)) return false;

    return true;
}

// board_clear()

void board_clear(board_t * board) {

    ASSERT(board!=nullptr);

    // edge squares
    for (int_fast32_t sq = 0; sq < SquareNb; sq++) {
      board->square[sq] = Edge;
   }

    // empty squares
    for (int_fast32_t sq_64 = 0; sq_64 < 64; sq_64++) {
        int_fast32_t sq = SQUARE_FROM_64(sq_64);
        board->square[sq] = Empty;
    }

    // misc
    board->turn = ColourNone;
    board->flags = FlagsNone;
    board->ep_square = SquareNone;
    board->ply_nb = 0;
}

// board_copy()

void board_copy(board_t * dst, const board_t * src) {

   ASSERT(dst!=nullptr);
   ASSERT(board_is_ok(src));

   *dst = *src;
}

// board_init_list()

void board_init_list(board_t * board) {

    ASSERT(board!=nullptr);

    // init
    for (int_fast32_t sq = 0; sq < SquareNb; ++sq)
        board->pos[sq] = -1;

    board->piece_nb = 0;
    for (int_fast32_t piece = 0; piece < 12; piece++)
        board->number[piece] = 0;

    // piece lists
    for (int_fast8_t colour = 0; colour < ColourNb; colour++) {

        // piece list
        int_fast32_t pos = 0;
        board->piece_material[colour] = 0; // Thomas

        board->piece[colour].clear();
        for (int_fast32_t sq_64 = 0; sq_64 < 64; sq_64++) {

            int_fast32_t sq = SQUARE_FROM_64(sq_64), piece = board->square[sq];
            if (piece != Empty && !piece_is_ok(piece)) my_fatal("board_init_list(): illegal position. %d\n", __LINE__);

            if (COLOUR_IS(piece,colour) && !PIECE_IS_PAWN(piece)) {

                if (pos >= 16) my_fatal("board_init_list(): illegal position\n");
                ASSERT(pos>=0&&pos<16);

                board->pos[sq] = pos;
                board->piece[colour].push_back(sq);
                ++pos;

                board->piece_nb++;
                board->number[PIECE_TO_12(piece)]++;

                if (piece != WK && piece != BK) // Thomas
                    board->piece_material[colour] += VALUE_PIECE(piece);
            }
        }

        if (board->number[COLOUR_IS_WHITE(colour)?WhiteKing12:BlackKing12] != 1) my_fatal("board_init_list(): illegal position. %d\n", __LINE__);
        if (board->number[WhiteBishop12] >= 10) printf("illegal position!\n");

        ASSERT(pos>=1&&pos<=16);

        // MV sort
        int_fast32_t size = board->piece[colour].size();

        for (int_fast32_t i = 1; i < size; i++) {

            int_fast32_t square = board->piece[colour][i], piece = board->square[square], order = PIECE_ORDER(piece);

            int_fast32_t sq = 0;
            for (pos = i; pos > 0 && order > PIECE_ORDER(board->square[(sq=board->piece[colour][pos-1])]); pos--) {
                ASSERT(pos>0&&pos<size);
                board->piece[colour][pos] = sq;
                ASSERT(board->pos[sq]==pos-1);
                board->pos[sq] = pos;
            }

            ASSERT(pos>=0&&pos<size);
            board->piece[colour][pos] = square;
            ASSERT(board->pos[square]==i);
            board->pos[square] = pos;
        }

        // debug
        if (DEBUG) {

            for (int_fast32_t i = 0; i < board->piece[colour].size(); i++) {
                int_fast32_t sq = board->piece[colour][i];
                ASSERT(board->pos[sq]==i);

                if (i == 0) { // king
                    ASSERT(PIECE_IS_KING(board->square[sq]));
                } else {
                    ASSERT(!PIECE_IS_KING(board->square[sq]));
                    ASSERT(PIECE_ORDER(board->square[board->piece[colour][i]])<=PIECE_ORDER(board->square[board->piece[colour][i-1]]));
                }
            }
        }

        // pawn list

        for (int_fast32_t file = 0; file < FileNb; file++)
            board->pawn_file[colour][file] = 0;

        pos = 0;

        board->pawn[colour].clear();
        for (int_fast32_t sq_64 = 0; sq_64 < 64; sq_64++) {

            int_fast32_t sq = SQUARE_FROM_64(sq_64), piece = board->square[sq];

            if (COLOUR_IS(piece,colour) && PIECE_IS_PAWN(piece)) {

                if (pos >= 8 || SQUARE_IS_PROMOTE(sq)) my_fatal("board_init_list(): illegal position. %d\n", __LINE__);
                ASSERT(pos>=0&&pos<8);

                board->pos[sq] = pos;
                board->pawn[colour].push_back(sq);
                ++pos;

                board->piece_nb++;
                board->number[PIECE_TO_12(piece)]++;
                board->pawn_file[colour][SQUARE_FILE(sq)] |= BIT(PAWN_RANK(sq,colour));

                board->piece_material[colour] += VALUE_PIECE(piece); // Thomas
            }
        }

        ASSERT(pos>=0&&pos<=8);

        if (board->piece[colour].size() + board->pawn[colour].size() > 16)
            printf("%d %d\n",  board->piece[colour].size(), board->pawn[colour].size());
        if (board->piece[colour].size() + board->pawn[colour].size() > 16)
            my_fatal("board_init_list(): illegal position. %d\n", __LINE__);
    }

    // last square
    board->cap_sq = SquareNone;

    // PST
    board->opening = board_opening(board);
    board->endgame = board_endgame(board);

    // hash key
    for (int_fast32_t i = 0; i < board->ply_nb; i++) board->stack.assign(board->ply_nb, 0); // HACK

    board->key = hash_key(board);
    board->pawn_key = hash_pawn_key(board);
    board->material_key = hash_material_key(board);

    // legality
    if (!board_is_legal(board)) my_fatal("board_init_list(): illegal position. %d\n", __LINE__);

    // debug
    ASSERT(board_is_ok(board));
}

// board_is_legal()

bool board_is_legal(const board_t * board) {

    ASSERT(board!=nullptr);
    return !IS_IN_CHECK(board,COLOUR_OPP(board->turn));
}

// board_is_check()

bool board_is_check(const board_t * board) {

    ASSERT(board!=nullptr);
    return IS_IN_CHECK(board,board->turn);
}

// board_is_mate()

bool board_is_mate(const board_t * board) {

    ASSERT(board!=nullptr);

    attack_t attack[1];
    attack_set(attack,board);

    if (!ATTACK_IN_CHECK(attack)) return false; // not in check => not mate
    if (legal_evasion_exist(board,attack)) return false; // legal move => not mate

    return true; // in check and no legal move => mate
}

// board_is_stalemate()

bool board_is_stalemate(board_t * board) {

    ASSERT(board!=nullptr);

    // init
    if (IS_IN_CHECK(board,board->turn)) return false; // in check => not stalemate

    // move loop
    list_t list[1];
    gen_moves(list,board);

    for (int_fast32_t i = 0; i < LIST_SIZE(list); i++) {
        int_fast32_t move = LIST_MOVE(list,i);
        if (pseudo_is_legal(move,board)) return false; // legal move => not stalemate
    }

    return true; // in check and no legal move => mate
}

// board_is_repetition()

bool board_is_repetition(const board_t * board) {

    ASSERT(board!=nullptr);

    // 50-move rule
    if (board->ply_nb >= 100) { // potential draw
        if (board->ply_nb > 100) return true;
        ASSERT(board->ply_nb==100);
        return !board_is_mate(board);
    }

   // position repetition
    ASSERT(board->sp>=board->ply_nb);
   //z = 0;
    for (int_fast32_t i = 4; i <= board->ply_nb; i += 2) {
        if (board->stack[board->stack.size()-i] == board->key) return true;
        //if (board->stack[board->stack.size()-i] == board->key) z++;
        //if (z >= 2) return true; // value 2 for 3-Times Repetition
    }

    return false;
}

// board_opening()

int board_opening(const board_t * board) {

    ASSERT(board!=nullptr);

    int_fast32_t opening = 0;

    for (int_fast8_t colour = 0; colour < ColourNb; colour++) {

        for (auto ptr = board->piece[colour].begin(); ptr != board->piece[colour].end(); ++ptr)  {
            int_fast32_t piece = board->square[*ptr];
            opening += PST(PIECE_TO_12(piece),SQUARE_TO_64(*ptr),Opening);
        }

        for (auto ptr = board->pawn[colour].begin(); ptr != board->pawn[colour].end(); ++ptr) {
            int_fast32_t piece = board->square[*ptr];
            opening += PST(PIECE_TO_12(piece),SQUARE_TO_64(*ptr),Opening);
        }
    }

    return opening;
}

// board_endgame()

int board_endgame(const board_t * board) {

    ASSERT(board!=nullptr);
    int_fast32_t endgame = 0;

    for (int_fast8_t colour = 0; colour < ColourNb; colour++) {

        for (auto ptr = board->piece[colour].begin(); ptr != board->piece[colour].end(); ++ptr) {
            int_fast32_t piece = board->square[*ptr];
            endgame += PST(PIECE_TO_12(piece),SQUARE_TO_64(*ptr),Endgame);
        }

        for (auto ptr = board->pawn[colour].begin(); ptr != board->pawn[colour].end(); ++ptr) {
            int_fast32_t piece = board->square[*ptr];
            endgame += PST(PIECE_TO_12(piece),SQUARE_TO_64(*ptr),Endgame);
        }
    }

   return endgame;
}

// end of board.cpp

