
#ifndef VEC_H
#define VEC_H

#include "TinyStd.hpp"
#include <math.h>
#include <stdint.h>

using ts::Array;

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

  double length()
  {
    return sqrt(pow(this->x, 2) + pow(this->y, 2));
  }

  _Vec2 operator-(const _Vec2& other)
  {
    return _Vec2(this->x - other.x, this->y - other.y);
  }

  _Vec2 operator+(const _Vec2& other)
  {
    return _Vec2(this->x + other.x, this->y + other.y);
  }

  _Vec2 operator*(const _Vec2& other)
  {
    return _Vec2(this->x * other.x, this->y * other.y);
  }

  _Vec2 operator/(const _Vec2& other)
  {
    return _Vec2(this->x / other.x, this->y / other.y);
  }

  _Vec2 operator*(T value)
  {
    return _Vec2(this->x * value, this->y * value);
  }

  _Vec2 operator/(T value)
  {
    return _Vec2(this->x / value, this->y / value);
  }

  _Vec2 operator-=(const _Vec2& other)
  {
    this->x -= other.x;
    this->y -= other.y;
    return *this;
  }

  _Vec2 operator+=(const _Vec2& other)
  {
    this->x += other.x;
    this->y += other.y;
    return *this;
  }

  _Vec2 operator*=(const _Vec2& other)
  {
    this->x *= other.x;
    this->y *= other.y;
    return *this;
  }

  _Vec2 operator/=(const _Vec2& other)
  {
    this->x /= other.x;
    this->y /= other.y;
    return *this;
  }

  _Vec2 operator*=(T v)
  {
    this->x *= v;
    this->y *= v;
    return *this;
  }

  _Vec2 operator/=(T v)
  {
    this->x /= v;
    this->y /= v;
    return *this;
  }

  _Vec2 normalize()
  {
    return *this / this->length();
  }
};

using Vec2 = _Vec2<double>;
using Vec2i = _Vec2<int>;

class Mat4 {
  public:
  Array<float, 16> data;

  Mat4()
  {
    this->data.zeroFill();
  }

  static Mat4 Identity()
  {
    Mat4 mat = Mat4();
    mat.data[0] = 1;
    mat.data[5] = 1;
    mat.data[10] = 1;
    mat.data[15] = 1;
    return mat;
  }

  void multiply(Mat4 other)
  {
    float result[16] = { 0 };

    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        result[i * 4 + j] = data[i * 4 + 0] * other.data[0 * 4 + j] + data[i * 4 + 1] * other.data[1 * 4 + j]
            + data[i * 4 + 2] * other.data[2 * 4 + j] + data[i * 4 + 3] * other.data[3 * 4 + j];
      }
    }

    // Copy the result into current matrix
    for (int i = 0; i < 16; i++) {
      data[i] = result[i];
    }
  }

  // Apply translation (tx, ty, tz)
  void applyTranslation(float tx, float ty, float tz)
  {
    Mat4 translationMatrix = Mat4::Identity();

    translationMatrix.data[3] = tx; // Set translation values
    translationMatrix.data[7] = ty;
    translationMatrix.data[11] = tz;

    multiply(translationMatrix); // Multiply current matrix with translation
  }

  // Apply scaling (sx, sy, sz)
  void applyScaling(float sx, float sy, float sz)
  {
    Mat4 scalingMatrix = Mat4::Identity();

    scalingMatrix.data[0] = sx; // Set scaling values
    scalingMatrix.data[5] = sy;
    scalingMatrix.data[10] = sz;

    multiply(scalingMatrix); // Multiply current matrix with scaling
  }

  void print()
  {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        ts::print_nnl("{} ", data[i * 4 + j]);
      }
      ts::print("");
    }
  }
};

using Vec2 = _Vec2<double>;
using Vec2i = _Vec2<int>;

#endif // VEC_H