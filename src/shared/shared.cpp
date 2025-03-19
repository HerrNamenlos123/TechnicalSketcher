
#include "shared.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"

#include "platform/linux-platform.cpp"

#include "format.cpp"

void panicStr(const char* str)
{
  panic("{}", str);
}

void panicSizeT(const char* str, size_t value)
{
  panic(str, value);
}