// eval.cpp

// includes

#include <cstdlib> // for abs()

#include "attack.h"
#include "eval.h"
#include "material.h"
#include "move.h"
#include "option.h"
#include "pawn.h"
#include "see.h"
#include "value.h"

// macros

/*
BitBases
*/

#define ADD_PIECE(type)  {\
         egbb_piece[total_pieces] = type;\
         egbb_square[total_pieces] = from;\
        ++total_pieces;\
};
constexpr int_fast8_t MAX_PIECES = 32;

/*
EndBitbases
*/

#define THROUGH(piece) ((piece)==Empty)

// constants and variables

const std::array<std::array<int_fast8_t, 256>, 2> KnightOutpostMatrix = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 5, 10, 10, 5, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 5, 10, 10, 5, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 4, 5, 5, 4, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 4, 5, 5, 4, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 5, 10, 10, 5, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 5, 10, 10, 5, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static int_fast32_t lazy_eval_cutoff = 50;
static bool         KingSafety       = false; // true
//static int_fast32_t KingSafetyMargin = 1600;
static bool         king_is_safe[ColourNb];

static /* constexpr */ int_fast16_t PieceActivityWeight = 256; // 100%
static /* constexpr */ int_fast16_t ShelterOpening      = 256; // 100%
static /* constexpr */ int_fast16_t KingSafetyWeight    = 256; // 100%
static /* constexpr */ int_fast16_t PassedPawnWeight    = 256; // 100%

static constexpr int_fast8_t MobMove    = 1;
static constexpr int_fast8_t MobAttack  = 1;
static constexpr int_fast8_t MobDefense = 0;

static constexpr std::array<int_fast8_t, 9>  knight_mob     = {-16, -12, -8, -4, 0, 4, 8, 12, 16};
static constexpr std::array<int_fast8_t, 14> bishop_mob     = {-30, -25, -20, -15, -10, -5, 0, 5, 10, 15, 20, 25, 30,
															   35};
static constexpr std::array<int_fast8_t, 15> rook_mob_open  = {-14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12,
															   14};
static constexpr std::array<int_fast8_t, 15> rook_mob_end   = {-28, -24, -20, -16, -12, -8, -4, 0, 4, 8, 12, 16, 20, 24,
															   28};
static constexpr std::array<int_fast8_t, 27> queen_mob_open = {-13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1,
															   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
static constexpr std::array<int_fast8_t, 27> queen_mob_end  = {-26, -24, -22, -20, -18, -16, -14, -12, -10, -8, -6, -4,
															   -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26};

static constexpr std::array<int_fast8_t, 9> pawns_on_bishop_colour_opening = {9, 6, 3, 0, -3, -6, -9, -12, -15};
static constexpr std::array<int_fast8_t, 9> pawns_on_bishop_colour_endgame = {12, 8, 4, 0, -4, -8, -12, -16, -20};

static constexpr int_fast8_t RookSemiOpenFileOpening  = 10;
static constexpr int_fast8_t RookSemiOpenFileEndgame  = 10;
static constexpr int_fast8_t RookOpenFileOpening      = 20;
static constexpr int_fast8_t RookOpenFileEndgame      = 20;
static constexpr int_fast8_t RookSemiKingFileOpening  = 10;
static constexpr int_fast8_t RookKingFileOpening      = 20;
static constexpr int_fast8_t RookOnBadPawnFileOpening = 8;
static constexpr int_fast8_t RookOnBadPawnFileEndgame = 8;

static /* constexpr */ int_fast16_t KingAttackOpening = 20; // was 20

static constexpr int_fast8_t knight_tropism_opening = 3;
static constexpr int_fast8_t bishop_tropism_opening = 2;
static constexpr int_fast8_t rook_tropism_opening   = 2;
static constexpr int_fast8_t queen_tropism_opening  = 2;

static constexpr int_fast8_t knight_tropism_endgame = 3;
static constexpr int_fast8_t bishop_tropism_endgame = 1;
static constexpr int_fast8_t rook_tropism_endgame   = 1;
static constexpr int_fast8_t queen_tropism_endgame  = 4;

static /* constexpr */ int_fast16_t StormOpening = 10;

static constexpr int_fast8_t Rook7thOpening  = 20;
static constexpr int_fast8_t Rook7thEndgame  = 40;
static constexpr int_fast8_t Queen7thOpening = 10;
static constexpr int_fast8_t Queen7thEndgame = 20;

static constexpr int_fast16_t TrappedBishop = 100;

static constexpr int_fast8_t BlockedBishop     = 50;
static constexpr int_fast8_t BlockedRook       = 50;
static constexpr int_fast8_t BlockedCenterPawn = 10;

static constexpr int_fast8_t  PassedOpeningMin = 10;
static constexpr int_fast8_t  PassedOpeningMax = 70;
static constexpr int_fast8_t  PassedEndgameMin = 20;
static constexpr int_fast16_t PassedEndgameMax = 140;

static constexpr int_fast16_t UnstoppablePasser = 800;
static constexpr int_fast8_t  FreePasser        = 60;

static constexpr int_fast8_t AttackerDistance = 5;
static constexpr int_fast8_t DefenderDistance = 20;

// "constants"

static constexpr std::array<int_fast16_t, 16> KingAttackWeight = {
	0, 0, 128, 192, 224, 240, 248, 252, 254, 255, 256, 256, 256, 256, 256, 256,
};


// variables

static int_fast32_t MobUnit[ColourNb][PieceNb];

static int_fast32_t KingAttackUnit[PieceNb];

// prototypes

static void eval_draw(const board_t *board, const material_info_t *mat_info, const pawn_info_t *pawn_info,
					  int_fast32_t mul[2]);

static void eval_piece(const board_t *board, const material_info_t *mat_info, const pawn_info_t *pawn_info,
					   int_fast32_t *opening, int_fast32_t *endgame);
static void eval_king(const board_t *board, const material_info_t *mat_info, int_fast32_t *opening,
					  int_fast32_t *endgame);
static void eval_passer(const board_t *board, const pawn_info_t *pawn_info, int_fast32_t *opening,
						int_fast32_t *endgame);
static void eval_pattern(const board_t *board, int_fast32_t *opening, int_fast32_t *endgame);

static bool unstoppable_passer(const board_t *board, int_fast32_t pawn, int_fast8_t colour);
static bool king_passer(const board_t *board, int_fast32_t pawn, int_fast8_t colour);
static bool free_passer(const board_t *board, int_fast32_t pawn, int_fast8_t colour);

static int_fast32_t pawn_att_dist(int_fast32_t pawn, int_fast32_t king, int_fast8_t colour);
static int_fast32_t pawn_def_dist(int_fast32_t pawn, int_fast32_t king, int_fast8_t colour);

static void draw_init_list(int_fast32_t list[], const board_t *board, int_fast32_t pawn_colour);

static bool draw_krpkr(const int_fast32_t list[]);
static bool draw_kbpkb(const int_fast32_t list[]);

static int_fast32_t shelter_square(const board_t *board, int_fast32_t square, int_fast8_t colour);
static int_fast8_t  shelter_file(const board_t *board, int_fast32_t file, int_fast32_t rank, int_fast8_t colour);

static int_fast16_t storm_file(const board_t *board, int_fast32_t file, int_fast8_t colour);

static bool bishop_can_attack(const board_t *board, int_fast32_t to, int_fast8_t colour);

// functions

// eval_init()

void eval_init() {

	// UCI options

	PieceActivityWeight = (option_get_int("Piece Activity") * 256 + 50) / 100;
	//KingSafetyWeight	 = (option_get_int("King Safety")	 * 256 + 50) / 100;
	PassedPawnWeight    = (option_get_int("Passed Pawns") * 256 + 50) / 100;
	ShelterOpening      = (option_get_int("Pawn Shelter") * 256 + 50) / 100;
	StormOpening        = (10 * option_get_int("Pawn Storm")) / 100;
	KingAttackOpening   = (20 * option_get_int("King Attack")) / 100;

	if (option_get_int("Chess Knowledge") == 500) lazy_eval_cutoff = ValueEvalInf;
	else lazy_eval_cutoff = (50 * option_get_int("Chess Knowledge")) / 100;

	// mobility table

	for (int_fast8_t colour = 0; colour < ColourNb; ++colour) {
		for (int_fast16_t piece = 0; piece < PieceNb; ++piece) {
			MobUnit[colour][piece] = 0;
		}
	}

	MobUnit[White][Empty] = MobMove;

	MobUnit[White][BP] = MobAttack;
	MobUnit[White][BN] = MobAttack;
	MobUnit[White][BB] = MobAttack;
	MobUnit[White][BR] = MobAttack;
	MobUnit[White][BQ] = MobAttack;
	MobUnit[White][BK] = MobAttack;

	MobUnit[White][WP] = MobDefense;
	MobUnit[White][WN] = MobDefense;
	MobUnit[White][WB] = MobDefense;
	MobUnit[White][WR] = MobDefense;
	MobUnit[White][WQ] = MobDefense;
	MobUnit[White][WK] = MobDefense;

	MobUnit[Black][Empty] = MobMove;

	MobUnit[Black][WP] = MobAttack;
	MobUnit[Black][WN] = MobAttack;
	MobUnit[Black][WB] = MobAttack;
	MobUnit[Black][WR] = MobAttack;
	MobUnit[Black][WQ] = MobAttack;
	MobUnit[Black][WK] = MobAttack;

	MobUnit[Black][BP] = MobDefense;
	MobUnit[Black][BN] = MobDefense;
	MobUnit[Black][BB] = MobDefense;
	MobUnit[Black][BR] = MobDefense;
	MobUnit[Black][BQ] = MobDefense;
	MobUnit[Black][BK] = MobDefense;

	// KingAttackUnit[]

	for (int_fast16_t piece = 0; piece < PieceNb; ++piece) {
		KingAttackUnit[piece] = 0;
	}

	KingAttackUnit[WN] = 1;
	KingAttackUnit[WB] = 1;
	KingAttackUnit[WR] = 2;
	KingAttackUnit[WQ] = 4;

	KingAttackUnit[BN] = 1;
	KingAttackUnit[BB] = 1;
	KingAttackUnit[BR] = 2;
	KingAttackUnit[BQ] = 4;
}

// eval()

int_fast32_t eval(/*const*/ board_t *board, int_fast32_t alpha, int_fast32_t beta, bool do_le, bool in_check) {

	ASSERT(board != NULL);

	ASSERT(board_is_legal(board));
	ASSERT(!board_is_check(board)); // exceptions are extremely rare

	// material
	material_info_t mat_info[1];
	material_get_info(mat_info, board);

	/*
	BitBases
	*/
	if (egbb_is_loaded && (mat_info->flags & MatBitbaseFlag) != 0) {

		const int_fast8_t player       = board->turn;
		int_fast8_t       total_pieces = 2;
		int_fast32_t      egbb_piece[MAX_PIECES], egbb_square[MAX_PIECES];
		for (int_fast32_t i            = 0; i < MAX_PIECES; ++i) {
			egbb_piece[i]  = 0;
			egbb_square[i] = 0;
		}
		egbb_piece[0]                  = _WKING;
		egbb_piece[1]                  = _BKING;
		for (int_fast8_t from = 0; from < 64; ++from) {
			if (board->square[SQUARE_FROM_64(from)] == Empty) continue;

			switch (board->square[SQUARE_FROM_64(from)]) {
				case WP: ADD_PIECE(_WPAWN);
					break;
				case BP: ADD_PIECE(_BPAWN);
					break;
				case WN: ADD_PIECE(_WKNIGHT);
					break;
				case BN: ADD_PIECE(_BKNIGHT);
					break;
				case WB: ADD_PIECE(_WBISHOP);
					break;
				case BB: ADD_PIECE(_BBISHOP);
					break;
				case WR: ADD_PIECE(_WROOK);
					break;
				case BR: ADD_PIECE(_BROOK);
					break;
				case WQ: ADD_PIECE(_WQUEEN);
					break;
				case BQ: ADD_PIECE(_BQUEEN);
					break;
				case WK:
					egbb_square[0] = from;
					break;
				case BK:
					egbb_square[1] = from;
					break;
				default:
					break;
			}
		}

		const int_fast32_t score = probe_egbb(player, egbb_piece, egbb_square);

		if (score != _NOTFOUND) {
			if (score == 0)
				return ValueDraw;
			else
				return score;
		}
	}

	/*
	End bitbases
	*/

	int_fast32_t opening = mat_info->opening, endgame = mat_info->endgame;

	int_fast32_t mul[ColourNb];
	mul[White] = mat_info->mul[White];
	mul[Black] = mat_info->mul[Black];

	// PST
	opening += board->opening;
	endgame += board->endgame;

	// draw
	pawn_info_t pawn_info[1];
	pawn_get_info(pawn_info, board);
	eval_draw(board, mat_info, pawn_info, mul);

	if (mat_info->mul[White] < mul[White]) mul[White] = mat_info->mul[White];
	if (mat_info->mul[Black] < mul[Black]) mul[Black] = mat_info->mul[Black];

	if (mul[White] == 0 && mul[Black] == 0) return ValueDraw;

	bool is_cut = false;

/*
	// Tempo
	if (COLOUR_IS_WHITE(board->turn))  {
	opening += 20;
	endgame += 10;
	} else{
	opening -= 20;
	endgame -= 10;
	}
*/

	const int_fast32_t phase     = mat_info->phase;
	int_fast32_t       lazy_eval = ((opening * (256 - mat_info->phase)) + (endgame * mat_info->phase)) / 256;

	if (COLOUR_IS_BLACK(board->turn)) lazy_eval = -lazy_eval;

	/* lazy Cutoff */
	if (do_le && !in_check && board->piece[White].size() > 2 && board->piece[Black].size() > 2) {

		ASSERT(eval >= -ValueEvalInf && eval <= +ValueEvalInf);

		//if (lazy_eval - lazy_eval_cutoff >= beta)
		//return (lazy_eval);
		if (lazy_eval + board->pvalue + lazy_eval_cutoff <= alpha) {
			//return (lazy_eval+board->pvalue);
			is_cut = true;
			goto cut;
		}
		// end lazy cuttoff
	}

	// pawns
	//pawn_get_info(pawn_info,board); moved earlier

	opening += pawn_info->opening;
	endgame += pawn_info->endgame;

	// eval
	eval_piece(board, mat_info, pawn_info, &opening, &endgame);
	eval_king(board, mat_info, &opening, &endgame);
	eval_passer(board, pawn_info, &opening, &endgame);
	eval_pattern(board, &opening, &endgame);

	cut:

	// phase mix
	//phase = mat_info->phase;
	int_fast32_t eval = ((opening * (256 - phase)) + (endgame * phase)) / 256;

	// drawish bishop endgames
	if ((mat_info->flags & DrawBishopFlag) != 0) {

		int_fast8_t wb = board->piece[White][1];
		ASSERT(PIECE_IS_BISHOP(board->square[wb]));

		int_fast8_t bb = board->piece[Black][1];
		ASSERT(PIECE_IS_BISHOP(board->square[bb]));

		if (SQUARE_COLOUR(wb) != SQUARE_COLOUR(bb)) {
			if (mul[White] == 16) mul[White] = 8; // 1/2
			if (mul[Black] == 16) mul[Black] = 8; // 1/2
		}
	}

	// draw bound
	if (eval > ValueDraw) {
		eval = (eval * mul[White]) / 16;
	} else if (eval < ValueDraw) {
		eval = (eval * mul[Black]) / 16;
	}

	// value range
	if (eval < -ValueEvalInf) eval = -ValueEvalInf;
	if (eval > +ValueEvalInf) eval = +ValueEvalInf;

	ASSERT(eval >= -ValueEvalInf && eval <= +ValueEvalInf);

	// turn
	if (COLOUR_IS_BLACK(board->turn)) eval = -eval;
	if (!is_cut) board->pvalue = abs(eval - lazy_eval);

	ASSERT(!value_is_mate(eval));

	// Tempo
	int_fast8_t tempo = 10;//((10 * (256 - phase)) + (20 * phase)) / 256;

	// Tempo draw bound
	// TODO: sign error? move me earlier?
	if (COLOUR_IS_WHITE(board->turn)) {
		if (eval > ValueDraw) {
			tempo = (tempo * mul[White]) / 16;
		} else if (eval < ValueDraw) {
			tempo = (tempo * mul[Black]) / 16;
		}
	} else {
		if (eval < ValueDraw) {
			tempo = (tempo * mul[White]) / 16;
		} else if (eval > ValueDraw) {
			tempo = (tempo * mul[Black]) / 16;
		}
	}

	return (eval + tempo);
}

// eval_draw()

static void eval_draw(const board_t *board, const material_info_t *mat_info, const pawn_info_t *pawn_info,
					  int_fast32_t mul[2]) {

	ASSERT(board != NULL);
	ASSERT(mat_info != NULL);
	ASSERT(pawn_info != NULL);
	ASSERT(mul != NULL);

	// draw patterns

	for (int_fast8_t colour = 0; colour < ColourNb; ++colour) {

		const int_fast8_t me = colour, opp = COLOUR_OPP(me);

		// KB*P+K* draw
		if ((mat_info->cflags[me] & MatRookPawnFlag) != 0) {

			const int_fast16_t pawn = pawn_info->single_file[me];

			if (pawn != SquareNone) { // all pawns on one file

				const int_fast8_t pawn_file = SQUARE_FILE(pawn);
				if (pawn_file == FileA || pawn_file == FileH) {

					const int_fast16_t king = KING_POS(board, opp);
					const int_fast8_t  prom = PAWN_PROMOTE(pawn, me);

					if (DISTANCE(king, prom) <= 1 && !bishop_can_attack(board, prom, me)) {
						mul[me] = 0;
					}
				}
			}
		}

		// K(B)P+K+ draw
		if ((mat_info->cflags[me] & MatBishopFlag) != 0) {

			const int_fast16_t pawn = pawn_info->single_file[me];

			if (pawn != SquareNone) { // all pawns on one file

				const int_fast16_t king = KING_POS(board, opp);

				if (SQUARE_FILE(king) == SQUARE_FILE(pawn)
					&& PAWN_RANK(king, me) > PAWN_RANK(pawn, me)
					&& !bishop_can_attack(board, king, me)) {
					mul[me] = 1; // 1/16
				}
			}
		}

		// KNPK* draw
		if ((mat_info->cflags[me] & MatKnightFlag) != 0) {

			const int_fast16_t pawn = board->pawn[me][0], king = KING_POS(board, opp);

			if (SQUARE_FILE(king) == SQUARE_FILE(pawn)
				&& PAWN_RANK(king, me) > PAWN_RANK(pawn, me)
				&& PAWN_RANK(pawn, me) <= Rank6) {
				mul[me] = 1; // 1/16
			}
		}
	}

	// recognisers, only heuristic draws here!
	if (false) {
	} else if (mat_info->recog == MAT_KRPKR) {

		int_fast32_t list[7 + 1];
		// KRPKR (white)
		draw_init_list(list, board, White);

		if (draw_krpkr(list)) {
			mul[White] = 1; // 1/16;
			mul[Black] = 1; // 1/16;
		}
	} else if (mat_info->recog == MAT_KRKRP) {

		int_fast32_t list[7 + 1];
		// KRPKR (black)
		draw_init_list(list, board, Black);

		if (draw_krpkr(list)) {
			mul[White] = 1; // 1/16;
			mul[Black] = 1; // 1/16;
		}
	} else if (mat_info->recog == MAT_KBPKB) {

		int_fast32_t list[7 + 1];
		// KBPKB (white)
		draw_init_list(list, board, White);

		if (draw_kbpkb(list)) {
			mul[White] = 1; // 1/16;
			mul[Black] = 1; // 1/16;
		}
	} else if (mat_info->recog == MAT_KBKBP) {

		int_fast32_t list[7 + 1];
		// KBPKB (black)
		draw_init_list(list, board, Black);

		if (draw_kbpkb(list)) {
			mul[White] = 1; // 1/16;
			mul[Black] = 1; // 1/16;
		}
	}
}

// eval_piece()

static void eval_piece(const board_t *board, const material_info_t *mat_info, const pawn_info_t *pawn_info,
					   int_fast32_t *opening, int_fast32_t *endgame) {

	ASSERT(board != NULL);
	ASSERT(mat_info != NULL);
	ASSERT(pawn_info != NULL);
	ASSERT(opening != NULL);
	ASSERT(endgame != NULL);

	// init
	int_fast32_t op[ColourNb], eg[ColourNb];
	op[0] = op[1] = eg[0] = eg[1] = 0;

	// eval

	for (int_fast8_t colour = 0; colour < ColourNb; ++colour) {

		const int_fast8_t  me    = colour, opp = COLOUR_OPP(me);
		//const int_fast32_t opp_flag = COLOUR_FLAG(opp);
		const int_fast32_t *unit = MobUnit[me];

		// piece loop
		for (auto from = board->piece[me].begin() + 1; from != board->piece[me].end(); ++from) { // HACK: no king

			const int_fast16_t piece = board->square[*from];
			int_fast32_t       mob;
			int_fast16_t       out_mob, capture;

			const int_fast16_t king       = KING_POS(board, opp);
			const int_fast8_t  king_file  = SQUARE_FILE(king), king_rank = SQUARE_RANK(king);
			const int_fast8_t  piece_file = SQUARE_FILE(*from), piece_rank = SQUARE_RANK(*from);
			switch (PIECE_TYPE(piece)) {

				case Knight64:

					// mobility
					mob = 0;

					mob += unit[board->square[*from - 33]];
					mob += unit[board->square[*from - 31]];
					mob += unit[board->square[*from - 18]];
					mob += unit[board->square[*from - 14]];
					mob += unit[board->square[*from + 14]];
					mob += unit[board->square[*from + 18]];
					mob += unit[board->square[*from + 31]];
					mob += unit[board->square[*from + 33]];

					op[me] += knight_mob[mob];
					eg[me] += knight_mob[mob];

					op[me] += (knight_tropism_opening * 8) -
							  (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * knight_tropism_opening));
					eg[me] += (knight_tropism_endgame * 8) -
							  (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * knight_tropism_endgame));


					// outpost
					out_mob = 0;
					if (me == White) {
						if (board->square[*from - 17] == WP)
							out_mob += KnightOutpostMatrix[me][*from];
						if (board->square[*from - 15] == WP)
							out_mob += KnightOutpostMatrix[me][*from];
					}
					else {
						if (board->square[*from + 17] == BP)
							out_mob += KnightOutpostMatrix[me][*from];
						if (board->square[*from + 15] == BP)
							out_mob += KnightOutpostMatrix[me][*from];
					}

					op[me] += out_mob;
					// eg[me] += out_mob;

					break;

				case Bishop64:

					// mobility
					mob = 0;

					for (int_fast16_t to = *from - 17; capture = board->square[to], THROUGH(capture); to -= 17)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from - 15; capture = board->square[to], THROUGH(capture); to -= 15)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from + 15; capture = board->square[to], THROUGH(capture); to += 15)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from + 17; capture = board->square[to], THROUGH(capture); to += 17)
						mob += MobMove;
					mob += unit[capture];

					op[me] += bishop_mob[mob];
					eg[me] += bishop_mob[mob];

					op[me] += (bishop_tropism_opening * 8) -
							  (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * bishop_tropism_opening));
					eg[me] += (bishop_tropism_endgame * 8) -
							  (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * bishop_tropism_endgame));

					if (SQUARE_COLOUR(*from) == White) {
						op[me] += pawns_on_bishop_colour_opening[pawn_info->wsp[me]];
						eg[me] += pawns_on_bishop_colour_endgame[pawn_info->wsp[me]];
					} else {
						if (me == White) {
							op[me] += pawns_on_bishop_colour_opening[(board->number[WhitePawn12] - pawn_info->wsp[me])];
							eg[me] += pawns_on_bishop_colour_endgame[(board->number[WhitePawn12] - pawn_info->wsp[me])];
						} else {
							op[me] += pawns_on_bishop_colour_opening[(board->number[BlackPawn12] - pawn_info->wsp[me])];
							eg[me] += pawns_on_bishop_colour_endgame[(board->number[BlackPawn12] - pawn_info->wsp[me])];
						}
					}

					break;

				case Rook64:

					// mobility

					mob = 0;

					for (int_fast16_t to = *from - 16; capture = board->square[to], THROUGH(capture); to -= 16)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from - 1; capture = board->square[to], THROUGH(capture); to -= 1)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from + 1; capture = board->square[to], THROUGH(capture); to += 1)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from + 16; capture = board->square[to], THROUGH(capture); to += 16)
						mob += MobMove;
					mob += unit[capture];

					op[me] += rook_mob_open[mob];
					eg[me] += rook_mob_end[mob];

					op[me] += (rook_tropism_opening * 8) -
							  (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * rook_tropism_opening));
					eg[me] += (rook_tropism_endgame * 8) -
							  (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * rook_tropism_endgame));

					// open file
					op[me] -= RookOpenFileOpening / 2;
					eg[me] -= RookOpenFileEndgame / 2;

					if (board->pawn_file[me][piece_file] == 0) { // no friendly pawn

						op[me] += RookSemiOpenFileOpening;
						eg[me] += RookSemiOpenFileEndgame;

						if (board->pawn_file[opp][piece_file] == 0) { // no enemy pawn
							op[me] += RookOpenFileOpening - RookSemiOpenFileOpening;
							eg[me] += RookOpenFileEndgame - RookSemiOpenFileEndgame;
						} else {

							const int_fast32_t BadPawnFile =
												   1 << (piece_file - FileA); // HACK: see BadPawnFileA and FileA
							if ((pawn_info->badpawns[opp] & BadPawnFile) != 0) {
								op[me] += RookOnBadPawnFileOpening;
								eg[me] += RookOnBadPawnFileEndgame;
							}
						}

						if ((mat_info->cflags[opp] & MatKingFlag) != 0) {

							int_fast16_t delta = abs(piece_file - king_file); // file distance

							if (delta <= 1) {
								op[me] += RookSemiKingFileOpening;
								if (delta == 0) op[me] += RookKingFileOpening - RookSemiKingFileOpening;
							}
						}
					}

					// 7th rank

					if (PAWN_RANK(*from, me) == Rank7) {
						if ((pawn_info->flags[opp] & BackRankFlag) != 0 // opponent pawn on 7th rank
							|| PAWN_RANK(KING_POS(board, opp), me) == Rank8) {
							op[me] += Rook7thOpening;
							eg[me] += Rook7thEndgame;
						}
					}

					break;

				case Queen64:

					// mobility

					mob = 0;

					for (int_fast16_t to = *from - 17; capture = board->square[to], THROUGH(capture); to -= 17)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from - 16; capture = board->square[to], THROUGH(capture); to -= 16)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from - 15; capture = board->square[to], THROUGH(capture); to -= 15)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from - 1; capture = board->square[to], THROUGH(capture); to -= 1)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from + 1; capture = board->square[to], THROUGH(capture); to += 1)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from + 15; capture = board->square[to], THROUGH(capture); to += 15)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from + 16; capture = board->square[to], THROUGH(capture); to += 16)
						mob += MobMove;
					mob += unit[capture];

					for (int_fast16_t to = *from + 17; capture = board->square[to], THROUGH(capture); to += 17)
						mob += MobMove;
					mob += unit[capture];

					op[me] += queen_mob_open[mob];
					eg[me] += queen_mob_end[mob];

					op[me] += (queen_tropism_opening * 8) -
							  (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * queen_tropism_opening));
					eg[me] += (queen_tropism_endgame * 8) -
							  (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * queen_tropism_endgame));

					// 7th rank

					if (PAWN_RANK(*from, me) == Rank7) {
						if ((pawn_info->flags[opp] & BackRankFlag) != 0 // opponent pawn on 7th rank
							|| PAWN_RANK(KING_POS(board, opp), me) == Rank8) {
							op[me] += Queen7thOpening;
							eg[me] += Queen7thEndgame;
						}
					}

					break;
			}
		}
	}

	// update
	*opening += ((op[White] - op[Black]) * PieceActivityWeight) / 256;
	*endgame += ((eg[White] - eg[Black]) * PieceActivityWeight) / 256;
}

// eval_king()

static void eval_king(const board_t *board, const material_info_t *mat_info, int_fast32_t *opening,
					  int_fast32_t *endgame) {

	ASSERT(board != NULL);
	ASSERT(mat_info != NULL);
	ASSERT(opening != NULL);
	ASSERT(endgame != NULL);

	// init
	int_fast32_t op[ColourNb], eg[ColourNb];
	op[0] = op[1] = eg[0] = eg[1] = 0;

	// white pawn shelter
	if ((mat_info->cflags[White] & MatKingFlag) != 0) {

		/* Thomas simple pattern king safety */
		king_is_safe[White] = false;

		if (KingSafety) {
			if (board->square[G1] == WK || board->square[H1] == WK) {
				if (board->square[G2] == WP && (board->square[H2] == WP || board->square[H3] == WP)) {
					king_is_safe[White] = true;
				} else if (board->square[F2] == WP && board->square[G3] == WP && board->square[H2] == WP) {
					king_is_safe[White] = true;
				}
			} else if (board->square[B1] == WK || board->square[A1] == WK) {
				if (board->square[B2] == WP && (board->square[A2] == WP || board->square[A3] == WP)) {
					king_is_safe[White] = true;
				} else if (board->square[C2] == WP && board->square[B3] == WP && board->square[A2] == WP) {
					king_is_safe[White] = true;
				}
			}
		}

		if (king_is_safe[White] == false) {
			const int_fast8_t  me        = White;
			// king
			const int_fast16_t penalty_1 = shelter_square(board, KING_POS(board, me), me);

			// castling
			int_fast16_t penalty_2 = penalty_1;

			if ((board->flags & FlagsWhiteKingCastle) != 0) {
				const int_fast16_t tmp = shelter_square(board, G1, me);
				if (tmp < penalty_2) penalty_2 = tmp;
			}

			if ((board->flags & FlagsWhiteQueenCastle) != 0) {
				const int_fast16_t tmp = shelter_square(board, B1, me);
				if (tmp < penalty_2) penalty_2 = tmp;
			}

			ASSERT(penalty_2 >= 0 && penalty_2 <= penalty_1);

			// penalty
			const int_fast16_t penalty = (penalty_1 + penalty_2) / 2;
			ASSERT(penalty >= 0);

			op[me] -= (penalty * ShelterOpening) / 256;
		}
	}

	// black pawn shelter
	if ((mat_info->cflags[Black] & MatKingFlag) != 0) {

		king_is_safe[Black] = false;

		if (KingSafety) {
			if (board->square[G8] == BK || board->square[H8] == BK) {
				if (board->square[G7] == BP && (board->square[H7] == BP || board->square[H6] == BP)) {
					king_is_safe[Black] = true;
				} else if (board->square[F7] == BP && board->square[G6] == BP && board->square[H7] == BP) {
					king_is_safe[Black] = true;
				}
			} else if (board->square[B8] == BK || board->square[A8] == BK) {
				if (board->square[B7] == BP && (board->square[A7] == BP || board->square[A6] == BP)) {
					king_is_safe[Black] = true;
				} else if (board->square[C7] == BP && board->square[B6] == BP && board->square[A7] == BP) {
					king_is_safe[Black] = true;
				}
			}
		}

		if (king_is_safe[Black] == false) {

			const int_fast8_t me = Black;

			// king
			const int_fast16_t penalty_1 = shelter_square(board, KING_POS(board, me), me);

			// castling
			int_fast16_t penalty_2 = penalty_1;

			if ((board->flags & FlagsBlackKingCastle) != 0) {
				const int_fast16_t tmp = shelter_square(board, G8, me);
				if (tmp < penalty_2) penalty_2 = tmp;
			}

			if ((board->flags & FlagsBlackQueenCastle) != 0) {
				const int_fast16_t tmp = shelter_square(board, B8, me);
				if (tmp < penalty_2) penalty_2 = tmp;
			}

			ASSERT(penalty_2 >= 0 && penalty_2 <= penalty_1);

			// penalty
			const int_fast16_t penalty = (penalty_1 + penalty_2) / 2;
			ASSERT(penalty >= 0);

			op[me] -= (penalty * ShelterOpening) / 256;
		}
	}

	// king attacks

	for (int_fast8_t colour = 0; colour < ColourNb; ++colour) {

		if ((mat_info->cflags[colour] & MatKingFlag) != 0) {

			const int_fast8_t  me   = colour, opp = COLOUR_OPP(me);
			const int_fast16_t king = KING_POS(board, me);
			//const int_fast8_t king_file = SQUARE_FILE(king), king_rank = SQUARE_RANK(king);

			// piece attacks
			int_fast16_t attack_tot = 0, piece_nb = 0;
			for (auto    from       = board->piece[opp].begin() + 1;
				 from != board->piece[opp].end(); ++from) { // HACK: no king

				const int_fast16_t piece = board->square[*from];

				if (piece_attack_king(board, piece, *from, king)) {
					++piece_nb;
					attack_tot += KingAttackUnit[piece];
				}
/*				 else{
					 if ((abs(king_file-SQUARE_FILE(*from)) + abs(king_rank-SQUARE_RANK(*from))) <= 4)  {
						++piece_nb;
						attack_tot += KingAttackUnit[piece];
					}
				} */
			}

			// scoring
			ASSERT(piece_nb >= 0 && piece_nb < 16);
			op[colour] -= (attack_tot * KingAttackOpening * KingAttackWeight[piece_nb]) / 256;

		}
	}

	// update
	*opening += (op[White] - op[Black]);
	*endgame += (eg[White] - eg[Black]);
}

// eval_passer()

static void eval_passer(const board_t *board, const pawn_info_t *pawn_info, int_fast32_t *opening,
						int_fast32_t *endgame) {

	ASSERT(board != NULL);
	ASSERT(pawn_info != NULL);
	ASSERT(opening != NULL);
	ASSERT(endgame != NULL);

	// init
	int_fast32_t op[ColourNb], eg[ColourNb];
	op[0] = op[1] = eg[0] = eg[1] = 0;

	//int_fast8_t white_passed_nb = 0, black_passed_nb = 0;

	// passed pawns
	for (int_fast8_t colour = 0; colour < ColourNb; ++colour) {

		const int_fast8_t att = colour, def = COLOUR_OPP(att);

		for (int_fast32_t bits = pawn_info->passed_bits[att]; bits != 0; bits &= bits - 1) {

			const int_fast8_t file = BIT_FIRST(bits);
			ASSERT(file >= FileA && file <= FileH);

			const int_fast8_t rank = BIT_LAST(board->pawn_file[att][file]);
			ASSERT(rank >= Rank2 && rank <= Rank7);

			int_fast16_t sq = SQUARE_MAKE(file, rank);
			if (COLOUR_IS_BLACK(att)) sq = SQUARE_RANK_MIRROR(sq);

			ASSERT(PIECE_IS_PAWN(board->square[sq]));
			ASSERT(COLOUR_IS(board->square[sq], att));


			// opening scoring
			op[att] += quad(PassedOpeningMin, PassedOpeningMax, rank);

			// endgame scoring init
			int_fast16_t min = PassedEndgameMin, max = PassedEndgameMax, delta = max - min;
			ASSERT(delta > 0);

			// "dangerous" bonus
			if (board->piece[def].size() <= 1 // defender has no piece
				&& (unstoppable_passer(board, sq, att) || king_passer(board, sq, att))) {
				delta += UnstoppablePasser;
			} else if (free_passer(board, sq, att)) {
				delta += FreePasser;
			}

			// king-distance bonus
			delta -= pawn_att_dist(sq, KING_POS(board, att), att) * AttackerDistance;
			delta += pawn_def_dist(sq, KING_POS(board, def), att) * DefenderDistance;

			// endgame scoring
			eg[att] += min;
			if (delta > 0) eg[att] += quad(0, delta, rank);
		}
	}

	// update
	*opening += ((op[White] - op[Black]) * PassedPawnWeight) / 256;
	*endgame += ((eg[White] - eg[Black]) * PassedPawnWeight) / 256;
}

// eval_pattern()

static void eval_pattern(const board_t *board, int_fast32_t *opening, int_fast32_t *endgame) {

	ASSERT(board != NULL);
	ASSERT(opening != NULL);
	ASSERT(endgame != NULL);

	// trapped bishop (7th rank)

	if ((board->square[A7] == WB && board->square[B6] == BP)
		|| (board->square[B8] == WB && board->square[C7] == BP)) {
		*opening -= TrappedBishop;
		*endgame -= TrappedBishop;
	}

	if ((board->square[H7] == WB && board->square[G6] == BP)
		|| (board->square[G8] == WB && board->square[F7] == BP)) {
		*opening -= TrappedBishop;
		*endgame -= TrappedBishop;
	}

	if ((board->square[A2] == BB && board->square[B3] == WP)
		|| (board->square[B1] == BB && board->square[C2] == WP)) {
		*opening += TrappedBishop;
		*endgame += TrappedBishop;
	}

	if ((board->square[H2] == BB && board->square[G3] == WP)
		|| (board->square[G1] == BB && board->square[F2] == WP)) {
		*opening += TrappedBishop;
		*endgame += TrappedBishop;
	}

	// trapped bishop (6th rank)

	if (board->square[A6] == WB && board->square[B5] == BP) {
		*opening -= TrappedBishop / 2;
		*endgame -= TrappedBishop / 2;
	}

	if (board->square[H6] == WB && board->square[G5] == BP) {
		*opening -= TrappedBishop / 2;
		*endgame -= TrappedBishop / 2;
	}

	if (board->square[A3] == BB && board->square[B4] == WP) {
		*opening += TrappedBishop / 2;
		*endgame += TrappedBishop / 2;
	}

	if (board->square[H3] == BB && board->square[G4] == WP) {
		*opening += TrappedBishop / 2;
		*endgame += TrappedBishop / 2;
	}

	// blocked bishop

	if (board->square[D2] == WP && board->square[D3] != Empty && board->square[C1] == WB) {
		*opening -= BlockedBishop;
	}

	if (board->square[E2] == WP && board->square[E3] != Empty && board->square[F1] == WB) {
		*opening -= BlockedBishop;
	}

	if (board->square[D7] == BP && board->square[D6] != Empty && board->square[C8] == BB) {
		*opening += BlockedBishop;
	}

	if (board->square[E7] == BP && board->square[E6] != Empty && board->square[F8] == BB) {
		*opening += BlockedBishop;
	}

	// blocked rook

	if ((board->square[C1] == WK || board->square[B1] == WK)
		&& (board->square[A1] == WR || board->square[A2] == WR || board->square[B1] == WR)) {
		*opening -= BlockedRook;
	}

	if ((board->square[F1] == WK || board->square[G1] == WK)
		&& (board->square[H1] == WR || board->square[H2] == WR || board->square[G1] == WR)) {
		*opening -= BlockedRook;
	}

	if ((board->square[C8] == BK || board->square[B8] == BK)
		&& (board->square[A8] == BR || board->square[A7] == BR || board->square[B8] == BR)) {
		*opening += BlockedRook;
	}

	if ((board->square[F8] == BK || board->square[G8] == BK)
		&& (board->square[H8] == BR || board->square[H7] == BR || board->square[G8] == BR)) {
		*opening += BlockedRook;
	}

	// White center pawn blocked
	if (board->square[E2] == BP && board->square[E3] != Empty) {
		*opening -= BlockedCenterPawn;
	}
	if (board->square[D2] == BP && board->square[D3] != Empty) {
		*opening -= BlockedCenterPawn;
	}

	// Black center pawn blocked
	if (board->square[E7] == BP && board->square[E6] != Empty) {
		*opening += BlockedCenterPawn;
	}
	if (board->square[D7] == BP && board->square[D6] != Empty) {
		*opening += BlockedCenterPawn;
	}

}

// unstoppable_passer()

static bool unstoppable_passer(const board_t *board, int_fast32_t pawn, int_fast8_t colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(COLOUR_IS_OK(colour));

	const int_fast8_t me = colour, opp = COLOUR_OPP(me);

	const int_fast8_t file = SQUARE_FILE(pawn), king = KING_POS(board, opp);
	int_fast8_t       rank = PAWN_RANK(pawn, me);
	// clear promotion path?

	for (auto sq = board->piece[me].begin(); sq != board->piece[me].end(); ++sq) {
		if (SQUARE_FILE(*sq) == file && PAWN_RANK(*sq, me) > rank) {
			return false; // "friendly" blocker
		}
	}

	// init

	if (rank == Rank2) {
		pawn += PAWN_MOVE_INC(me);
		++rank;
		ASSERT(rank == PAWN_RANK(pawn, me));
	}

	ASSERT(rank >= Rank3 && rank <= Rank7);

	const int_fast16_t prom = PAWN_PROMOTE(pawn, me);
	int_fast16_t       dist = DISTANCE(pawn, prom);
	ASSERT(dist == Rank8 - rank);

	if (board->turn == opp) ++dist;

	if (DISTANCE(king, prom) > dist) return true; // not in the square

	return false;
}

// king_passer()

static bool king_passer(const board_t *board, int_fast32_t pawn, int_fast8_t colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(COLOUR_IS_OK(colour));

	const int_fast16_t king = KING_POS(board, colour), file = SQUARE_FILE(pawn), prom = PAWN_PROMOTE(pawn, colour);

	if (DISTANCE(king, prom) <= 1
		&& DISTANCE(king, pawn) <= 1
		&& (SQUARE_FILE(king) != file
			|| (file != FileA && file != FileH))) {
		return true;
	}

	return false;
}

// free_passer()

static bool free_passer(const board_t *board, int_fast32_t pawn, int_fast8_t colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(COLOUR_IS_OK(colour));

	const int_fast16_t inc = PAWN_MOVE_INC(colour), sq = pawn + inc;
	ASSERT(SQUARE_IS_OK(sq));

	if (board->square[sq] != Empty) return false;

	const int_fast32_t move = MOVE_MAKE(pawn, sq);
	if (see_move(move, board) < 0) return false;

	return true;
}

// pawn_att_dist()

static int_fast32_t pawn_att_dist(int_fast32_t pawn, int_fast32_t king, int_fast8_t colour) {

	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(SQUARE_IS_OK(king));
	ASSERT(COLOUR_IS_OK(colour));

	const int_fast8_t inc = PAWN_MOVE_INC(colour);

	const int_fast16_t target = pawn + inc;

	return DISTANCE(king, target);
}

// pawn_def_dist()

static int_fast32_t pawn_def_dist(int_fast32_t pawn, int_fast32_t king, int_fast8_t colour) {

	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(SQUARE_IS_OK(king));
	ASSERT(COLOUR_IS_OK(colour));

	const int_fast8_t  inc    = PAWN_MOVE_INC(colour);
	const int_fast16_t target = pawn + inc;

	return DISTANCE(king, target);
}

// draw_init_list()

static void draw_init_list(int_fast32_t list[], const board_t *board, int_fast32_t pawn_colour) {

	ASSERT(list != NULL);
	ASSERT(board != NULL);
	ASSERT(COLOUR_IS_OK(pawn_colour));

	// init
	const int_fast8_t att = pawn_colour, def = COLOUR_OPP(att);

	ASSERT(board->pawn_size[att] == 1);
	ASSERT(board->pawn_size[def] == 0);

	int_fast32_t pos = 0;

	// att
	for (auto sq = board->piece[att].begin(); sq != board->piece[att].end(); ++sq) {
		list[pos++] = *sq;
	}

	for (auto sq = board->pawn[att].begin(); sq != board->pawn[att].end(); ++sq) {
		list[pos++] = *sq;
	}

	// def
	for (auto sq = board->piece[def].begin(); sq != board->piece[def].end(); ++sq) {
		list[pos++] = *sq;
	}

	for (auto sq = board->pawn[def].begin(); sq != board->pawn[def].end(); ++sq) {
		list[pos++] = *sq;
	}

	// end marker

	ASSERT(pos == board->piece_nb);

	list[pos] = SquareNone;

	// file flip?

	const int_fast16_t pawn = board->pawn[att][0];

	if (SQUARE_FILE(pawn) >= FileE) {
		for (int_fast16_t i = 0; i < pos; ++i) {
			list[i] = SQUARE_FILE_MIRROR(list[i]);
		}
	}

	// rank flip?

	if (COLOUR_IS_BLACK(pawn_colour)) {
		for (int_fast16_t i = 0; i < pos; ++i) {
			list[i] = SQUARE_RANK_MIRROR(list[i]);
		}
	}
}

// draw_krpkr()

static bool draw_krpkr(const int_fast32_t list[]) {

	ASSERT(list != NULL);
	ASSERT(COLOUR_IS_OK(turn));

	// load
	int_fast16_t wk, wr, wp, bk, br;
	wk = *list++;
	ASSERT(SQUARE_IS_OK(wk));

	wr = *list++;
	ASSERT(SQUARE_IS_OK(wr));

	wp = *list++;
	ASSERT(SQUARE_IS_OK(wp));
	ASSERT(SQUARE_FILE(wp) <= FileD);

	bk = *list++;
	ASSERT(SQUARE_IS_OK(bk));

	br = *list++;
	ASSERT(SQUARE_IS_OK(br));

	ASSERT(*list == SquareNone);

	// test

	const int_fast8_t wp_file = SQUARE_FILE(wp), wp_rank = SQUARE_RANK(wp);
	const int_fast8_t bk_file = SQUARE_FILE(bk), bk_rank = SQUARE_RANK(bk);
	const int_fast8_t br_file = SQUARE_FILE(br), br_rank = SQUARE_RANK(br);

	const int_fast8_t prom = PAWN_PROMOTE(wp, White);

	if (false) {
	} else if (bk == prom) {
		// TODO: rook near Rank1 if wp_rank == Rank6?
		if (br_file > wp_file) return true;
	} else if (bk_file == wp_file && bk_rank > wp_rank) {
		return true;
	} else if (wr == prom && wp_rank == Rank7 && (bk == G7 || bk == H7) && br_file == wp_file) {
		if (br_rank <= Rank3) {
			if (DISTANCE(wk, wp) > 1) return true;
		} else { // br_rank >= Rank4
			if (DISTANCE(wk, wp) > 2) return true;
		}
	}

	return false;
}

// draw_kbpkb()

static bool draw_kbpkb(const int_fast32_t list[]) {

	ASSERT(list != NULL);
	ASSERT(COLOUR_IS_OK(turn));

	// load
	int_fast16_t wk, wb, wp, bk, bb;
	wk = *list++;
	ASSERT(SQUARE_IS_OK(wk));

	wb = *list++;
	ASSERT(SQUARE_IS_OK(wb));

	wp = *list++;
	ASSERT(SQUARE_IS_OK(wp));
	ASSERT(SQUARE_FILE(wp) <= FileD);

	bk = *list++;
	ASSERT(SQUARE_IS_OK(bk));

	bb = *list++;
	ASSERT(SQUARE_IS_OK(bb));

	ASSERT(*list == SquareNone);

	// opposit colour?
	if (SQUARE_COLOUR(wb) == SQUARE_COLOUR(bb)) return false; // TODO

	// blocked pawn?
	const int_fast8_t  inc = PAWN_MOVE_INC(White);
	const int_fast16_t end = PAWN_PROMOTE(wp, White) + inc;

	for (int_fast16_t to = wp + inc; to != end; to += inc) {

		ASSERT(SQUARE_IS_OK(to));

		if (to == bb) return true; // direct blockade

		const int_fast16_t delta = to - bb;
		ASSERT(delta_is_ok(delta));

		if (PSEUDO_ATTACK(BB, delta)) {

			const int_fast8_t inc_2 = DELTA_INC_ALL(delta);
			ASSERT(inc_2 != IncNone);

			int_fast16_t sq = bb;
			do {
				sq += inc_2;
				ASSERT(SQUARE_IS_OK(sq));
				ASSERT(sq != wk);
				ASSERT(sq != wb);
				ASSERT(sq != wp);
				ASSERT(sq != bb);
				if (sq == to) return true; // indirect blockade
			} while (sq != bk);
		}
	}

	return false;
}

// shelter_square()

static int_fast32_t shelter_square(const board_t *board, int_fast32_t square, int_fast8_t colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(square));
	ASSERT(COLOUR_IS_OK(colour));

	const int_fast8_t file = SQUARE_FILE(square), rank = PAWN_RANK(square, colour);

	int_fast32_t penalty = (shelter_file(board, file, rank, colour) * 2);
	if (file != FileA) penalty += (shelter_file(board, file - 1, rank, colour));
	if (file != FileH) penalty += (shelter_file(board, file + 1, rank, colour));

	if (penalty == 0) penalty = 11; // weak back rank

	penalty += storm_file(board, file, colour);
	if (file != FileA) penalty += storm_file(board, file - 1, colour);
	if (file != FileH) penalty += storm_file(board, file + 1, colour);

	return penalty;
}

// shelter_file()

static int_fast8_t shelter_file(const board_t *board, int_fast32_t file, int_fast32_t rank, int_fast8_t colour) {

	ASSERT(board != NULL);
	ASSERT(file >= FileA && file <= FileH);
	ASSERT(rank >= Rank1 && rank <= Rank8);
	ASSERT(COLOUR_IS_OK(colour));

	int_fast8_t dist = BIT_FIRST(board->pawn_file[colour][file] & BitGE[rank]);
	ASSERT(dist >= Rank2 && dist <= Rank8);

	dist = Rank8 - dist;
	ASSERT(dist >= 0 && dist <= 6);

	int_fast8_t penalty = 36 - dist * dist;
	ASSERT(penalty >= 0 && penalty <= 36);

	return penalty;
}

// storm_file()

static int_fast16_t storm_file(const board_t *board, int_fast32_t file, int_fast8_t colour) {

	ASSERT(board != NULL);
	ASSERT(file >= FileA && file <= FileH);
	ASSERT(COLOUR_IS_OK(colour));

	int_fast8_t dist = BIT_LAST(board->pawn_file[COLOUR_OPP(colour)][file]);
	ASSERT(dist >= Rank1 && dist <= Rank7);

	int_fast16_t penalty = 0;

	switch (dist) {
		case Rank4:
			penalty = StormOpening * 1;
			break;
		case Rank5:
			penalty = StormOpening * 3;
			break;
		case Rank6:
			penalty = StormOpening * 6;
			break;
	}

	return penalty;
}

// bishop_can_attack()

static bool bishop_can_attack(const board_t *board, int_fast32_t to, int_fast8_t colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(to));
	ASSERT(COLOUR_IS_OK(colour));

	for (auto from = board->piece[colour].begin() + 1; from != board->piece[colour].end(); ++from) { // HACK: no king
		int_fast8_t piece = board->square[*from];
		if (PIECE_IS_BISHOP(piece) && SQUARE_COLOUR(*from) == SQUARE_COLOUR(to))
			return true;
	}
	return false;
}

// end of eval.cpp
