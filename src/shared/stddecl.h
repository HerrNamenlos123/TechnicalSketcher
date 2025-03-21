
#ifndef STDDECL_H
#define STDDECL_H

#include <stddef.h>
#include <stdint.h>

struct Arena;
struct String;

void panicStr(const char* str);
void panicSizeT(const char* str, size_t value);
void __panicImpl();

void __format_vsnprintf(char* buffer, size_t bufferSize, const char* format, ...);
size_t __format_strlen(const char* str);

#endif // STDDECL_H