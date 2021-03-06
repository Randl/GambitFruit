// option.h

#ifndef OPTION_H
#define OPTION_H

// includes

#include <cstdint>
#include "util.h"

// functions

extern void option_init();
extern void option_list();

extern bool option_set(const char var[], const char val[]);
extern const char *option_get(const char var[]);

extern bool option_get_bool(const char var[]);
extern S64 option_get_int(const char var[]);
extern const char *option_get_string(const char var[]);

#endif // !defined OPTION_H

// end of option.h

