
#include "stddecl.h"

template <typename... Args>
String format(Arena& arena, const char* fmt, Args&&... args);

template <typename... Args>
void print(const char* fmt, Args&&... args);

template <typename... Args>
void print_stderr(const char* fmt, Args&&... args);