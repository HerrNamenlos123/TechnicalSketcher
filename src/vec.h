
#ifndef VEC_H
#define VEC_H

#include <math.h>
#include <stdint.h>

template <typename T> class _Vec2 {
public:
  T x;
  T y;

  _Vec2()
  {
    this->x = 0;
    this->y = 0;
  }

  _Vec2(T x, T y)
  {
    this->x = x;
    this->y = y;
  }

  double length() { return sqrt(pow(this->x, 2) + pow(this->y, 2)); }

  _Vec2 operator-(const _Vec2& other) { return _Vec2(this->x - other.x, this->y - other.y); }

  _Vec2 operator+(const _Vec2& other) { return _Vec2(this->x + other.x, this->y + other.y); }

  _Vec2 operator*(T value) { return _Vec2(this->x * value, this->y * value); }

  _Vec2 operator/(T value) { return _Vec2(this->x / value, this->y / value); }
};

using Vec2 = _Vec2<double>;
using Vec2i = _Vec2<int>;

#endif // VEC_H