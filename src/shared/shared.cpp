
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

Result<int64_t, ParseError> strToInt(String string)
{
  char* endptr;
  StackArena<64> tmpArena;
  int64_t value = strtol(string.c_str(tmpArena), &endptr, 10);
  if (*endptr != '\0') {
    return ParseError::NonDigitsRemaining;
  }
  return value;
}