// book.cpp

// includes

#include <cerrno>
#include <cstdio>
#include <cstring>

#include "board.h"
#include "book.h"
#include "move.h"
#include "move_gen.h"

// types

struct book_entry_t {
    U64 key;
    U16 move;
    U16 count;
    U16 n;
    U16 sum;
};

// variables

static FILE *BookFile;
static S32 BookSize;

// prototypes

static S32 find_pos(U64 key);

static void read_entry(book_entry_t *entry, S32 n);
static U64 read_integer(FILE *file, S32 size);

// functions

// book_init()

void book_init() {
	BookFile = nullptr;
	BookSize = 0;
}

// book_open()

void book_open(const char file_name[]) {

	ASSERT(file_name != nullptr);

	BookFile = fopen(file_name, "rb");

	if (BookFile != nullptr) {
		if (fseek(BookFile, 0, SEEK_END) == -1)
			my_fatal("book_open(): fseek(): %s\n", strerror(errno));

		BookSize = ftell(BookFile) / 16;
		if (BookSize == -1) my_fatal("book_open(): ftell(): %s\n", strerror(errno));
	}
}

// book_close()

void book_close() {
	if (BookFile != nullptr && fclose(BookFile) == EOF)
		my_fatal("book_close(): fclose(): %s\n", strerror(errno));
}

// book_move()

S32 book_move(board_t *board) {

	ASSERT(board != nullptr);

	if (BookFile != nullptr && BookSize != 0) {

		// draw a move according to a fixed probability distribution

		S32 best_move = MoveNone;
		S32 best_score = 0;

		for (S32 pos = find_pos(board->key); pos < BookSize; ++pos) {

			book_entry_t entry[1];
			read_entry(entry, pos);
			if (entry->key != board->key) break;

			U16 move = entry->move;
			S32 score = entry->count;

			// pick this move?

			ASSERT(score > 0);

			best_score += score;
			if (my_random(best_score) < score) best_move = move;
		}

		if (best_move != MoveNone) {

			// convert PolyGlot move into Fruit move;
			list_t list[1];
			gen_legal_moves(list, board);

			for (S32 i = 0; i < list->size; ++i) {
				U16 move = list->moves[i].move;
				if (MOVE_IS_PROMOTE(move)) {  //promotion TODO: test
					if ((move & 07777 | ((MOVE_PROMOTE_PIECE(move) + 1) << 12)) == best_move)
						return move;
				} else if ((move & 07777) == best_move)
					return move;
			}
		}
	}

	return MoveNone;
}

// find_pos()

static S32 find_pos(U64 key) {

	// binary search (finds the leftmost entry)

	S32 left = 0, right = BookSize - 1;

	ASSERT(left <= right);

	book_entry_t entry[1];
	while (left < right) {

		S32 mid = (left + right) / 2;
		ASSERT(mid >= left && mid < right);

		read_entry(entry, mid);

		if (key <= entry->key)
			right = mid;
		else
			left = mid + 1;
	}

	ASSERT(left == right);

	read_entry(entry, left);

	return (entry->key == key) ? left : BookSize;
}

// read_entry()

static void read_entry(book_entry_t *entry, S32 n) {

	ASSERT(entry != nullptr);
	ASSERT(n >= 0 && n < BookSize);

	ASSERT(BookFile != nullptr);

	if (fseek(BookFile, n * 16, SEEK_SET) == -1)
		my_fatal("read_entry(): fseek(): %s\n", strerror(errno));

	entry->key = read_integer(BookFile, 8);
	entry->move = read_integer(BookFile, 2);
	entry->count = read_integer(BookFile, 2);
	entry->n = read_integer(BookFile, 2);
	entry->sum = read_integer(BookFile, 2);
}

// read_integer()

static U64 read_integer(FILE *file, S32 size) {

	ASSERT(file != nullptr);
	ASSERT(size > 0 && size <= 8);

	U64 n = 0;

	for (S32 i = 0; i < size; ++i) {

		S32 b = fgetc(file);

		if (b == EOF) {
			if (feof(file))
				my_fatal("read_integer(): fgetc(): EOF reached\n");
			else // error
				my_fatal("read_integer(): fgetc(): %s\n", strerror(errno));
		}

		ASSERT(b >= 0 && b < 256);
		n = (n << 8) | b;
	}

	return n;
}

// end of book.cpp
