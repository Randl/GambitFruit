
// list.cpp

// includes

#include "board.h"
#include "list.h"
#include "move.h"
#include "util.h"
#include "value.h"

// constants

static constexpr bool UseStrict = true;

// functions

// list_is_ok()

bool list_is_ok(const list_t * list) {

	if (list == nullptr) return false;
	if (list->size < 0 || list->size >= ListSize) return false;

	return true;
}

// list_remove()

void list_remove(list_t * list, int_fast32_t pos) {

	ASSERT(list_is_ok(list));
	ASSERT(pos>=0&&pos<list->size);

	for (int_fast32_t i = pos; i < list->size-1; ++i) {
		list->move[i] = list->move[i+1];
		list->value[i] = list->value[i+1];
	}

	list->size--;
}

// list_copy()

void list_copy(list_t * dst, const list_t * src) {

	ASSERT(dst!=nullptr);
	ASSERT(list_is_ok(src));

	dst->size = src->size;

	for (int_fast32_t i = 0; i < src->size; ++i) {
		dst->move[i] = src->move[i];
		dst->value[i] = src->value[i];
	}
}

// list_sort()

void list_sort(list_t * list) {

	ASSERT(list_is_ok(list));

	// init

	const int_fast32_t size = list->size;
	list->value[size] = -32768; // HACK: sentinel

   // insert sort (stable)

	for (int_fast32_t i = size-2; i >= 0; --i) {
		const int_fast32_t move = list->move[i], value = list->value[i];
		int_fast32_t j;
		for (j = i; value < list->value[j+1]; ++j) {
			list->move[j] = list->move[j+1];
			list->value[j] = list->value[j+1];
		}

		ASSERT(j<size);

		list->move[j] = move;
		list->value[j] = value;
	}

	// debug

	if (DEBUG) {
		for (int_fast32_t i = 0; i < size-1; ++i) {
			ASSERT(list->value[i]>=list->value[i+1]);
		}
	}
}

// list_contain()

bool list_contain(const list_t * list, int_fast32_t move) {

	ASSERT(list_is_ok(list));
	ASSERT(move_is_ok(move));

	for (int_fast32_t i = 0; i < list->size; ++i)
		if (list->move[i] == move) return true;

	return false;
}

// list_note()

void list_note(list_t * list) {

	ASSERT(list_is_ok(list));

	for (int_fast32_t i = 0; i < list->size; ++i) {
		const int_fast32_t move = list->move[i];
		ASSERT(move_is_ok(move));
		list->value[i] = -move_order(move);
	}
}

// list_filter()

void list_filter(list_t * list, board_t * board, move_test_t test, bool keep) {

	ASSERT(list!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(test!=nullptr);
	ASSERT(keep==true||keep==false);

	int_fast32_t pos = 0;

	for (int_fast32_t i = 0; i < LIST_SIZE(list); ++i) {

		ASSERT(pos>=0&&pos<=i);

		const int_fast32_t move = LIST_MOVE(list,i), value = LIST_VALUE(list,i);

		if ((*test)(move,board) == keep) {
			list->move[pos] = move;
			list->value[pos] = value;
			++pos;
		}
   }

	ASSERT(pos>=0&&pos<=LIST_SIZE(list));
	list->size = pos;

	// debug
	ASSERT(list_is_ok(list));
}

// end of list.cpp
