// list.h

#ifndef LIST_H
#define LIST_H

// includes

#include <cstdint>
#include "board.h"
#include "util.h"

// constants

constexpr S16 ListSize = 256;

// macros

#define LIST_CLEAR(list)     ((list)->size=0)
#define LIST_ADD(list, mv)    ((list)->moves[(list)->size++].move=(mv))

#define LIST_IS_EMPTY(list)  ((list)->size==0)
#define LIST_SIZE(list)      ((list)->size)

#define LIST_MOVE(list, pos)  ((list)->moves[pos].move)
#define LIST_VALUE(list, pos) ((list)->moves[pos].value)

// types
typedef bool (*move_test_t)(U16 move, board_t *board);

struct move_value {
    U16 move;
    S16 value;
};

struct list_t {
	std::array<move_value, ListSize> moves;
    S16 size;

	list_t() : size(0) { }

	list_t(const list_t &);
	list_t(const list_t *);
	list_t &operator=(const list_t &);

	void sort();
	void note();
	bool is_ok() const;
    void remove(U16 pos);
	void filter(board_t *board, move_test_t test, bool keep);
    bool contains(U16 move) const;
};

#endif // !defined LIST_H

// end of list.h
