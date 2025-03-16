
#ifndef VEC_H
#define VEC_H

#include <stdint.h>

class Vec2 {
public:
  double x;
  double y;

  Vec2()
  {
    this->x = 0;
    this->y = 0;
  }

  Vec2(double x, double y)
  {
    this->x = x;
    this->y = y;
  }
};

#endif // VEC_H