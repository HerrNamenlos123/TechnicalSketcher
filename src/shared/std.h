
#ifndef BASE_H
#define BASE_H

#include "stddecl.h"

#include "format.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define tsk_min(a, b) (((a) < (b)) ? (a) : (b))
#define tsk_max(a, b) (((a) > (b)) ? (a) : (b))

const auto DEFAULT_ARENA_SIZE = 16 * 1024 * 1024;
const auto MAX_PRINT_LINE_LENGTH = 4096;

struct ArenaChunk {
  ArenaChunk* nextChunk;
  size_t capacity;
  size_t used;
  // After here comes the data
  // dataPointer = chunkPointer + sizeof(ArenaChunk)
};

struct Arena {
  ArenaChunk* firstChunk;
  bool isStackArena;

  [[nodiscard]] static Arena
  create(size_t chunkSize = DEFAULT_ARENA_SIZE)
  {
    size_t allocSize = sizeof(ArenaChunk) + chunkSize;
    Arena newArena;
    newArena.firstChunk = (ArenaChunk*)calloc(allocSize, 1);
    newArena.isStackArena = false;
    newArena.__initialized = true;
    if (newArena.firstChunk == 0) {
      panicSizeT("Arena chunk allocation of size {} failed", allocSize);
    }
    newArena.firstChunk->capacity = chunkSize;
    print("Creating arena");
    return newArena;
  }

  [[nodiscard]] static Arena createFromBuffer(char* buffer, size_t bufferSize)
  {
    if (bufferSize <= sizeof(ArenaChunk)) {
      panicStr("Cannot create arena from buffer: Buffer too small");
    }
    ArenaChunk* chunk = (ArenaChunk*)buffer;
    chunk->nextChunk = 0;
    chunk->capacity = bufferSize - sizeof(ArenaChunk);
    chunk->used = 0;
    memset((uint8_t*)chunk + sizeof(ArenaChunk), 0, chunk->capacity);

    Arena arena;
    arena.firstChunk = chunk;
    arena.isStackArena = true;
    arena.__initialized = true;
    return arena;
  }

  void
  enlarge(ArenaChunk** lastChunk, size_t chunkSize = DEFAULT_ARENA_SIZE)
  {
    print("Warning: Arena was enlarged. Consider more short-lived arenas to prevent excessive memory usage.");
    if (!this->__initialized) {
      panicStr("Arena was not properly initialized");
    }
    if (this->isStackArena) {
      panicStr("Cannot enlarge a stack-based arena");
    }
    size_t allocSize = sizeof(ArenaChunk) + chunkSize;
    (*lastChunk)->nextChunk = (ArenaChunk*)calloc(allocSize, 1);
    if ((*lastChunk)->nextChunk == 0) {
      panicSizeT("Arena chunk allocation of size {} failed", allocSize);
    }
    (*lastChunk) = (*lastChunk)->nextChunk;
    (*lastChunk)->capacity = chunkSize;
  }

  template <typename T>
  [[nodiscard]] T* allocate(size_t elementCount = 1)
  {
    if (!this->__initialized) {
      panicStr("Arena was not properly initialized");
    }
    size_t size = elementCount * sizeof(T);
    ArenaChunk* lastChunk = this->firstChunk;
    while (lastChunk->nextChunk) {
      lastChunk = lastChunk->nextChunk;
    }

    if (lastChunk->capacity - lastChunk->used < size) {
      if (this->isStackArena) {
        panicSizeT("Stack Arena is not large enough for allocation of size {}", size);
      } else {
        this->enlarge(&lastChunk, tsk_max(DEFAULT_ARENA_SIZE, size));
      }
    }

    uint8_t* data = (uint8_t*)lastChunk + sizeof(ArenaChunk) + lastChunk->used;
    lastChunk->used += size;
    return (T*)data;
  }

  void free()
  {
    ArenaChunk* current = this->firstChunk;
    while (current) {
      ArenaChunk* next = current->nextChunk;
      ::free(current);
      current = next;
    }
    this->firstChunk = 0;
    this->isStackArena = false;
    this->__initialized = false;
  }

  void clearAndReinit()
  {
    if (!this->__initialized) {
      *this = Arena::create();
    } else {
      // Clear every page except the first, and reset the first page
      // This means when reiniting an arena, most of the time no allocation is necessary
      ArenaChunk* current = this->firstChunk->nextChunk;
      while (current) {
        ArenaChunk* next = current->nextChunk;
        ::free(current);
        current = next;
      }
      this->firstChunk->nextChunk = 0;
      this->firstChunk->used = 0;
      memset((uint8_t*)this->firstChunk + sizeof(ArenaChunk), 0, this->firstChunk->capacity);
    }
  }

  private:
  bool __initialized;
};

template <size_t Size>
// Note: The Arena is private and automatically converted, because the Arena cannot outlive the StackArena.
// Do not save the Arena from the StackArena out to a separate variable.
struct StackArena {
  char data[Size];

  [[nodiscard]] operator Arena&()
  {
    // Only valid as long as the address of this->data does not change
    if (!this->arenaInitialized) {
      this->_arena = Arena::createFromBuffer(this->data, Size);
      this->arenaInitialized = true;
    }
    return this->_arena;
  }

  private:
  Arena _arena = {};
  bool arenaInitialized = 0;
};

struct String {
  const char* data;
  size_t length;

  static const size_t npos = -1;

  [[nodiscard]] String concat(Arena& arena, String other)
  {
    char* buf = arena.allocate<char>(this->length + other.length + 1);
    memcpy(buf, this->data, this->length);
    memcpy(buf + this->length, other.data, other.length);
    return String(buf, this->length + other.length);
  }

  [[nodiscard]] size_t find(String delimiter, size_t startIndex)
  {
    if (delimiter.length == 0 || startIndex >= length) {
      return npos;
    }
    for (size_t i = startIndex; i <= length - delimiter.length; ++i) {
      if (memcmp(data + i, delimiter.data, delimiter.length) == 0) {
        return i;
      }
    }
    return npos;
  }

  [[nodiscard]] String substr(size_t startIndex, size_t count = npos) const
  {
    if (startIndex >= length) {
      return { nullptr, 0 };
    }

    if (count == npos || startIndex + count > length) {
      count = length - startIndex;
    }

    return { data + startIndex, count };
  }

  [[nodiscard]] char get(size_t index)
  {
    if (index >= this->length) {
      panicStr("String index access out of range");
    }
    return this->data[index];
  }

  [[nodiscard]] char operator[](size_t index)
  {
    return this->get(index);
  }

  [[nodiscard]] const char* c_str(Arena& arena)
  {
    String s = String::clone(arena, *this);
    return s.data;
  }

  [[nodiscard]] bool startsWith(String other) const
  {
    if (this->length < other.length) {
      return false;
    }
    for (size_t i = 0; i < other.length; i++) {
      if (this->data[i] != other.data[i]) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] bool endsWith(String other) const
  {
    if (this->length < other.length) {
      return false;
    }
    for (size_t i = 0; i < other.length; i++) {
      if (this->data[this->length - other.length + i] != other.data[i]) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] static String clone(Arena& arena, String string)
  {
    char* buf = arena.allocate<char>(string.length + 1);
    memcpy(buf, string.data, string.length);
    return String(buf, string.length);
  }

  [[nodiscard]] static String clone(Arena& arena, const char* str)
  {
    return String::clone(arena, str, strlen(str));
  }

  [[nodiscard]] static String clone(Arena& arena, const char* str, size_t length)
  {
    char* buf = arena.allocate<char>(length + 1);
    memcpy(buf, str, length);
    return String(buf, length);
  }

  [[nodiscard]] static String view(const char* str)
  {
    return String(str, strlen(str));
  }

  [[nodiscard]] static String view(const char* str, size_t length)
  {
    return String(str, length);
  }

  [[nodiscard]] bool operator==(String other)
  {
    if (this->length != other.length) {
      return false;
    }
    return 0 == memcmp(this->data, other.data, this->length);
  }

  private:
  String(const char* str, size_t length)
  {
    this->data = str;
    this->length = length;
  }
};

inline String operator""s(const char* str, size_t length)
{
  return String::view(str, length);
}

template <typename... Args>
void panic(const char* fmt, Args&&... args)
{
  Arena arena = Arena::create();
  String str = format(arena, fmt, args...);
  print_stderr("[FATAL] Thread panicked: {}", str);
  fflush(stderr);
  abort();
  arena.free();
}

template <typename T>
struct Optional {

  Optional()
  {
    this->_hasValue = false;
  }

  Optional(T value)
  {
    this->_hasValue = true;
    this->_value = value;
  }

  [[nodiscard]] T& value()
  {
    if (!this->_hasValue) {
      panic("Bad optional access");
    }
    return this->_value;
  }

  [[nodiscard]] bool hasValue()
  {
    return this->_hasValue;
  }

  [[nodiscard]] operator bool()
  {
    return this->hasValue();
  }

  [[nodiscard]] T& operator*()
  {
    return this->value();
  }

  [[nodiscard]] T* operator->()
  {
    return &this->_value;
  }

  private:
  T _value;
  bool _hasValue;
};

template <typename TVal, typename TErr>
struct Result {

  Result(TVal succ)
  {
    this->_value = succ;
    this->_hasValue = true;
  }

  Result(TErr err)
  {
    this->_error = err;
    this->_hasValue = false;
  }

  [[nodiscard]] TVal& unwrap()
  {
    if (!this->_hasValue) {
      panic("Bad result access");
    }
    return this->_value;
  }

  [[nodiscard]] Optional<TVal> value()
  {
    if (!this->_hasValue) {
      return {};
    }
    return this->_value;
  }

  [[nodiscard]] TVal value_or(TVal fallback)
  {
    if (!this->_hasValue) {
      return fallback;
    }
    return this->_value;
  }

  [[nodiscard]] Optional<TErr> error()
  {
    if (this->_hasValue) {
      return {};
    }
    return this->_error;
  }

  [[nodiscard]] TErr& unwrap_error()
  {
    if (this->_hasValue) {
      panic("Bad result access");
    }
    return this->_error;
  }

  [[nodiscard]] bool hasValue()
  {
    return this->_hasValue;
  }

  [[nodiscard]] operator bool()
  {
    return this->hasValue();
  }

  private:
  union {
    TVal _value;
    TErr _error;
  };
  bool _hasValue;
};

template <typename T>
struct ListElem {
  T data;
  ListElem<T>* nextElement;
};

template <typename T>
struct List {
  size_t length = { 0 };

  void push(Arena& arena, T element)
  {
    if (!this->firstElement) {
      this->firstElement = arena.allocate<ListElem<T>>();
      this->firstElement->data = element;
    } else {
      ListElem<T>* lastElement = this->firstElement;
      while (lastElement->nextElement) {
        lastElement = lastElement->nextElement;
      }
      lastElement->nextElement = arena.allocate<ListElem<T>>();
      lastElement->nextElement->data = element;
    }
    this->length++;
  }

  void pop()
  {
    if (!this->firstElement) {
      panic("Cannot pop from list: length = 0");
    }

    if (!this->firstElement->nextElement) {
      this->firstElement = 0;
      this->length = 0;
      return;
    }

    ListElem<T>* secondToLastElement = this->firstElement;
    while (secondToLastElement->nextElement->nextElement) {
      secondToLastElement = secondToLastElement->nextElement;
    }

    secondToLastElement->nextElement = 0;
    this->length--;
  }

  [[nodiscard]] T& get(size_t index)
  {
    if (index < 0 || index >= this->length) {
      panic("List index out of bounds: {} >= {}", index, this->length);
    }
    ListElem<T>* elem = this->firstElement;
    for (size_t i = 0; i < index; i++) {
      elem = elem->nextElement;
    }
    return elem->data;
  }

  [[nodiscard]] T& operator[](size_t index)
  {
    return this->get(index);
  }

  [[nodiscard]] T& back()
  {
    if (this->length == 0) {
      panic("Cannot get back element of a zero element list");
    }
    return this->get(this->length - 1);
  }

  [[nodiscard]] bool contains(T val)
  {
    for (auto& elem : *this) {
      if (elem == val) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] Optional<size_t> findIndex(T val)
  {
    size_t index = 0;
    for (auto& elem : *this) {
      if (elem == val) {
        return index;
      }
      index++;
    }
    return {};
  }

  struct Iterator {
    ListElem<T>* current;
    Iterator(ListElem<T>* node)
        : current(node)
    {
    }

    T& operator*()
    {
      return current->data;
    }

    Iterator& operator++()
    {
      current = current->nextElement;
      return *this;
    }

    bool operator!=(const Iterator& other) const
    {
      return current != other.current;
    }
  };

  Iterator begin()
  {
    return Iterator(this->firstElement);
  }

  Iterator end()
  {
    return Iterator(nullptr);
  }

  private:
  ListElem<T>* firstElement = { 0 };
};

template <typename T, size_t Size>
struct Array {
  [[nodiscard]] T& get(size_t index)
  {
    if (index < 0 || index >= this->length()) {
      panic("Array index out of bounds: {} >= {}", index, this->length());
    }
    return this->_data[index];
  }

  [[nodiscard]] T& operator[](size_t index)
  {
    return this->get(index);
  }

  [[nodiscard]] size_t length()
  {
    return Size;
  }

  void zeroFill()
  {
    memset(this->_data, 0, sizeof(this->_data));
  }

  private:
  T _data[Size];
};

template <typename T, typename U>
struct Pair {
  T first;
  U second;
};

#endif // BASE_H