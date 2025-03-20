
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

uint8_t hexToDigit(char letter)
{
  if (letter >= '0' && letter <= '9') {
    return letter - '0';
  } else if (letter >= 'a' && letter <= 'f') {
    return letter - 'a' + 10;
  } else if (letter >= 'A' && letter <= 'F') {
    return letter - 'A' + 10;
  } else {
    return 0;
  }
}

Color hexToColor(String hex)
{
  if (hex.length == 4) {
    uint8_t r = hexToDigit(hex[1]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[2]) + 16 * hexToDigit(hex[2]);
    uint8_t b = hexToDigit(hex[3]) + 16 * hexToDigit(hex[3]);
    uint8_t a = 255;
    return Color(r, g, b, a);
  } else if (hex.length == 5) {
    uint8_t r = hexToDigit(hex[1]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[2]) + 16 * hexToDigit(hex[2]);
    uint8_t b = hexToDigit(hex[3]) + 16 * hexToDigit(hex[3]);
    uint8_t a = hexToDigit(hex[4]) + 16 * hexToDigit(hex[4]);
    return Color(r, g, b, a);
  } else if (hex.length == 7) {
    uint8_t r = hexToDigit(hex[2]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[4]) + 16 * hexToDigit(hex[3]);
    uint8_t b = hexToDigit(hex[6]) + 16 * hexToDigit(hex[5]);
    uint8_t a = 255;
    return Color(r, g, b, a);
  } else if (hex.length == 9) {
    uint8_t r = hexToDigit(hex[2]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[4]) + 16 * hexToDigit(hex[3]);
    uint8_t b = hexToDigit(hex[6]) + 16 * hexToDigit(hex[5]);
    uint8_t a = hexToDigit(hex[8]) + 16 * hexToDigit(hex[7]);
    return Color(r, g, b, a);
  } else {
    return Color(0, 0, 0, 0);
  }
}