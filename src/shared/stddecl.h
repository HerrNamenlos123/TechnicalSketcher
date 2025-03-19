
#ifndef STDDECL_H
#define STDDECL_H

#include <stddef.h>

struct Arena;
struct String;

void panicStr(const char* str);
void panicSizeT(const char* str, size_t value);

#endif // STDDECL_H