
#ifndef STD_H
#define STD_H

#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <variant>
#include <vector>

template <typename T> using List = std::vector<T>;
using String = std::string;

template <typename... TArgs> using Variant = std::variant<TArgs...>;

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
