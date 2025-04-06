
#ifndef SHARED_H
#define SHARED_H

#include "TinyStd.hpp"
#include "clay.h"

#include "platform/platform.h"

using namespace ts::literals;

struct Color;
enum struct ParseError { NonDigitsRemaining };
[[nodiscard]] ts::Result<int64_t, ParseError> strToInt(ts::String string);
[[nodiscard]] uint8_t hexToDigit(char letter);
[[nodiscard]] Color hexToColor(ts::String hex);

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
    Clay_Color color = {
      .r = r,
      .g = g,
      .b = b,
      .a = a,
    };
    return color;
  }
};

#endif // SHARED_H