
#ifndef STD_H
#define STD_H

#include "app.h"
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace fs = std::filesystem;

template <typename T> using List = std::vector<T>;
using String = std::string;

template <typename... TArgs> using Variant = std::variant<TArgs...>;
template <typename... TArgs> using Tuple = std::tuple<TArgs...>;

template <typename T, typename U> using Map = std::unordered_map<T, U>;

template <typename T> using Optional = std::optional<T>;

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

// template <typename T> class List {
// public:
//   List()
//   {
//     this->capacity = 4;
//     this->data = new T[this->capacity * sizeof(T)];
//     this->count = 0;
//   }

//   ~List() { delete[] this->data; }

//   void reserve(size_t count)
//   {
//     if (count > this->capacity) {
//       this->capacity = count;
//       this->data = realloc(this->data, this->capacity * sizeof(T));
//       if (!this->data) {
//         abort();
//       }
//     }
//   }

//   void resize(size_t count)
//   {
//     this->reserve(count);
//     this->count = count;
//   }

//   void push(T elem)
//   {
//     if (this->count + 1 >= this->capacity) {
//       this->reserve(this->capacity * 2);
//     }
//     this->data[this->count] = elem;
//     this->count++;
//   }

//   T pop()
//   {
//     T elem = this->data[this->count - 1];
//     this->count--;
//     return elem;
//   }

//   T back() { return this->data[this->count - 1]; }

// private:
//   T* data = 0;
//   size_t capacity = 0;
//   size_t count = 0;
// };

#endif // STD_H
