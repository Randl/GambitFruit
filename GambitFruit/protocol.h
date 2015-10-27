// protocol.h

#ifndef PROTOCOL_H
#define PROTOCOL_H

// includes

#include "util.h"

// functions

extern void loop();
extern void event();

extern void init();

extern void get(char string[], S32 size);
extern void send(const char format[], ...);

extern void parse_setoption(char string[]);

#endif // !defined PROTOCOL_H

// end of protocol.h

