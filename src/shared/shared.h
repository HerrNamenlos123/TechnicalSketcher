
#ifndef SHARED_H
#define SHARED_H

#include "clay.h"
#include "std.h"

#include "platform/platform.h"

struct Color;
enum struct ParseError {
  NonDigitsRemaining
};
[[nodiscard]] Result<int64_t, ParseError> strToInt(String string);
[[nodiscard]] uint8_t hexToDigit(char letter);
[[nodiscard]] Color hexToColor(String hex);

struct Color {
  float r = 0;
  float g = 0;
  float b = 0;
  float a = 0;

  Color()
  {
    this->r = 0;
    this->g = 0;
    this->b = 0;
    this->a = 0;
  }

  Color(float r, float g, float b, float a = 255)
  {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }

  Color(Clay_Color color)
  {
    this->r = color.r;
    this->g = color.g;
    this->b = color.b;
    this->a = color.a;
  }

  Color(String color)
  {
    *this = hexToColor(color);
  }

  Color(const char* color)
  {
    *this = hexToColor(String::view(color));
  }

  operator Clay_Color()
  {
    return (Clay_Color) {
      .r = r,
      .g = g,
      .b = b,
      .a = a,
    };
  }
};

#endif // SHARED_H