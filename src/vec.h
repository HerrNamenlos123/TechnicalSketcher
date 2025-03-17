
#ifndef VEC_H
#define VEC_H

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
};

using Vec2 = _Vec2<double>;
using Vec2i = _Vec2<int>;

#endif // VEC_H