
#ifndef STD_H
#define STD_H

#include "stddecl.h"

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
  bool __initialized;

  [[nodiscard]] static Arena
  create(size_t chunkSize = DEFAULT_ARENA_SIZE);

  [[nodiscard]] static Arena createFromBuffer(char* buffer, size_t bufferSize);

  void
  enlarge(ArenaChunk** lastChunk, size_t chunkSize = DEFAULT_ARENA_SIZE);

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

  void free();
  void clearAndReinit();
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

  [[nodiscard]] String concat(Arena& arena, String other);

  [[nodiscard]] size_t find(String delimiter, size_t startIndex);

  [[nodiscard]] String substr(size_t startIndex, size_t count = npos) const;

  [[nodiscard]] char get(size_t index);

  [[nodiscard]] char operator[](size_t index);

  [[nodiscard]] const char* c_str(Arena& arena);

  [[nodiscard]] bool startsWith(String other) const;

  [[nodiscard]] bool endsWith(String other) const;

  [[nodiscard]] static String clone(Arena& arena, String string);

  [[nodiscard]] static String clone(Arena& arena, const char* str);

  [[nodiscard]] static String clone(Arena& arena, const char* str, size_t length);

  [[nodiscard]] static String view(const char* str);

  [[nodiscard]] static String view(const char* str, size_t length);

  [[nodiscard]] bool operator==(String other);

  String()
      : data(0)
      , length(0)
  {
  }

  private:
  String(const char* str, size_t length)
  {
    this->data = str;
    this->length = length;
  }
};

inline String operator""_s(const char* str, size_t length)
{
  return String::view(str, length);
}

template <typename... Args>
void panic(const char* fmt, Args&&... args)
{
  Arena arena = Arena::create();
  String str = format(arena, fmt, args...);
  print_stderr("[FATAL] Thread panicked: {}", str);
  __panicImpl();
  arena.free();
}

template <typename T>
struct Optional {

  Optional()
      : _hasValue(false)
  {
  }

  Optional(T value)
      : _value(value)
      , _hasValue(true)
  {
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

  void clear()
  {
    this->length = 0;
    this->firstElement = 0;
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
  [[nodiscard]] T* data()
  {
    return this->_data;
  }

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

#endif // STD_H