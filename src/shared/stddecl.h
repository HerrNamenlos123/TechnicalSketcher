
#ifndef STDDECL_H
#define STDDECL_H

#include <stddef.h>
#include <stdint.h>

struct Arena;
struct String;

void panicStr(const char* str);
void panicSizeT(const char* str, size_t value);
void __panicImpl(String str);
void* __memset(void* dest, int ch, size_t count);

void __format_vsnprintf(char* buffer, size_t bufferSize, const char* format, ...);
size_t __format_strlen(const char* str);
void __format_output_stdout(String string);
void __format_output_stderr(String string);

#endif // STDDECL_H