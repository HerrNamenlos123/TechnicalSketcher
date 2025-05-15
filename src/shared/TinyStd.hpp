
#ifndef TINYSTD_H
#define TINYSTD_H

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#if defined(TINYSTD_USE_CLAY) && !defined(CLAY_HEADER)
#error TINYSTD_USE_CLAY is defined, but CLAY_HEADER not. This means that you want to use the Clay library bindings, but clay.h was not yet included. You must manually include clay.h BEFORE TinyStd.hpp.
#endif

namespace ts {

#define ts_min(a, b) (((a) < (b)) ? (a) : (b))
#define ts_max(a, b) (((a) > (b)) ? (a) : (b))

struct String;

void __panicStr(const char* str);
void __panicSizeT(const char* str, size_t value);
void __panicImpl(String str);
void* __memset(void* dest, int ch, size_t count);

void __format_vsnprintf(char* buffer, size_t bufferSize, const char* format, ...);
size_t __format_strlen(const char* str);
void __format_output_stdout(String string);
void __format_output_stderr(String string);

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

  [[nodiscard]] static Arena create(size_t chunkSize = DEFAULT_ARENA_SIZE);

  [[nodiscard]] static Arena createFromBuffer(char* buffer, size_t bufferSize);

  void enlarge(ArenaChunk** lastChunk, size_t chunkSize = DEFAULT_ARENA_SIZE);

  template <typename T> [[nodiscard]] T* allocate(size_t elementCount = 1)
  {
    if (!this->__initialized) {
      __panicStr("Arena was not properly initialized");
    }
    size_t size = elementCount * sizeof(T);
    ArenaChunk* lastChunk = this->firstChunk;
    while (lastChunk->nextChunk) {
      lastChunk = lastChunk->nextChunk;
    }

    if (lastChunk->capacity - lastChunk->used < size) {
      if (this->isStackArena) {
        __panicSizeT("Stack Arena is not large enough for allocation of size {}", size);
      } else {
        this->enlarge(&lastChunk, ts_max(DEFAULT_ARENA_SIZE, size));
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

  template <typename T> [[nodiscard]] T* allocate(size_t elementCount = 1)
  {
    Arena& arena = *this;
    return arena.allocate<T>(elementCount);
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

namespace literals {
  inline String operator""_s(const char* str, size_t length)
  {
    return String::view(str, length);
  }
}

template <typename... Args> void panic(const char* fmt, Args&&... args)
{
  Arena arena = Arena::create();
  String str = format(arena, fmt, args...);
  __panicImpl(str);
  arena.free();
}

template <typename T> struct Optional {

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

template <typename TVal, typename TErr> struct Result {

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

template <typename T> struct ListElem {
  T data;
  ListElem<T>* nextElement;
};

template <typename T> struct List {
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

  template <typename TFunc> void remove_if(TFunc&& pred)
  {
    while (firstElement && pred(firstElement->data)) {
      firstElement = firstElement->nextElement;
      length--;
    }

    ListElem<T>* curr = firstElement;
    while (curr && curr->nextElement) {
      if (pred(curr->nextElement->data)) {
        curr->nextElement = curr->nextElement->nextElement;
        length--;
      } else {
        curr = curr->nextElement;
      }
    }
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

template <typename T, size_t Size> struct Array {
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
    __memset(this->_data, 0, sizeof(this->_data));
  }

  private:
  T _data[Size];
};

template <typename T, typename U> struct Pair {
  T first;
  U second;
};

// ===========================================================
// ===                      FORMATTING                     ===
// ===========================================================

const int FORMAT_MAX_FORMATTER_LENGTH = 20;

template <typename T> struct remove_cv_ref {
  using type = T;
};

template <typename T> struct remove_cv_ref<const T> {
  using type = T;
};

template <typename T> struct remove_cv_ref<volatile T> {
  using type = T;
};

template <typename T> struct remove_cv_ref<const volatile T> {
  using type = T;
};

template <typename T> struct remove_cv_ref<T&> {
  using type = T;
};

template <typename T> struct remove_cv_ref<const T&> {
  using type = T;
};

template <typename T> struct remove_cv_ref<volatile T&> {
  using type = T;
};

template <typename T> struct remove_cv_ref<const volatile T&> {
  using type = T;
};

// Custom is_same implementation that ignores const, volatile, and references
template <typename T, typename U> struct is_same {
  static const bool value = false;
};

template <typename T> struct is_same<T, T> {
  static const bool value = true;
};

template <typename T, typename U>
inline constexpr bool is_same_v = is_same<typename remove_cv_ref<T>::type, typename remove_cv_ref<U>::type>::value;

struct false_type {
  static constexpr bool value = false;
};
struct true_type {
  static constexpr bool value = true;
};

template <typename T>
concept no_formatter = requires(T t) { T::value; };
template <typename T> struct formatter : false_type { };

template <> struct formatter<int> {
  static size_t format(const int& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%d", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<float> {
  static size_t format(const float& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%f", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<double> {
  static size_t format(const double& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%lf", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<unsigned long> {
  static size_t format(const unsigned long& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%lu", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<bool> {
  static size_t format(const bool& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    if (value) {
      __format_vsnprintf(buffer, remainingBufferSize, "true");
      return __format_strlen(buffer);
    } else {
      __format_vsnprintf(buffer, remainingBufferSize, "false");
      return __format_strlen(buffer);
    }
  }
};

template <> struct formatter<String> {
  static size_t format(const String& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    for (size_t i = 0; i < value.length; i++) {
      if (i < remainingBufferSize) {
        buffer[i] = value.data[i];
      }
    }
    return value.length;
  }
};

template <> struct formatter<const char*> {
  static size_t format(const char* const& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%s", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<char*> {
  static size_t format(char* value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%s", value);
    return __format_strlen(buffer);
  }
};

template <size_t LENGTH> struct formatter<char[LENGTH]> {
  static size_t format(const char value[LENGTH], String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%s", value);
    return __format_strlen(buffer);
  }
};

template <typename T> size_t format_value(const T& value, String formatArg, char* buffer, size_t remainingBufferSize)
{
  using formatter_t = formatter<typename remove_cv_ref<T>::type>;
  if constexpr (no_formatter<formatter_t>) {
    static_assert(!no_formatter<formatter_t>, "No formatter is defined for this datatype");
    return 0;
  } else {
    return formatter_t::format(value, formatArg, buffer, remainingBufferSize);
  }
}

template <typename... Args>
void format_impl(String formatStr, int currentArg, int desiredArg, char* buf, size_t bufsize, size_t& currentLength)
{
}

template <typename T, typename... Args>
void format_impl(String formatStr, int currentArg, int desiredArg, char* buf, size_t bufsize, size_t& currentLength,
    T&& value, Args&&... args)
{
  if (currentArg == desiredArg) {
    size_t size = format_value(value, formatStr, buf + currentLength, bufsize - currentLength);
    currentLength += size;
  } else {
    format_impl(formatStr, currentArg + 1, desiredArg, buf, bufsize, currentLength, args...);
  }
}

inline void __format_concat(char* buf, size_t& buflen, size_t maxBufferLength, char* src, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    if (buflen < maxBufferLength) {
      buf[buflen++] = src[i];
    }
  }
}

template <size_t MaxSize = 8192, typename... Args> String format(Arena& arena, const char* fmt, Args&&... args)
{
  char buf[MaxSize];
  size_t buflen = 0;
  char formatStr[FORMAT_MAX_FORMATTER_LENGTH];
  size_t formatStrLength = 0;
  size_t argIndex = 0;

  bool isParsingArg = false;
  size_t fmtLen = __format_strlen(fmt);
  for (size_t i = 0; i < fmtLen; i++) {
    char c = fmt[i];
    char cnext = i != fmtLen - 1 ? fmt[i + 1] : '\0';
    if (!isParsingArg) {
      if (c == '{' && cnext != '{') {
        isParsingArg = true;
        formatStrLength = 0;
      } else if (c == '{' && cnext == '{') {
        __format_concat(buf, buflen, MaxSize, &c, 1);
        i++;
      } else if (c == '}' && cnext == '}') {
        __format_concat(buf, buflen, MaxSize, &c, 1);
        i++;
      } else {
        __format_concat(buf, buflen, MaxSize, &c, 1);
      }
    } else {
      if (c == '}') {
        isParsingArg = false;
        format_impl(String::view(formatStr, formatStrLength), 0, argIndex, buf, MaxSize, buflen, args...);
        argIndex++;
      } else {
        if (formatStrLength < FORMAT_MAX_FORMATTER_LENGTH) {
          formatStr[formatStrLength++] = fmt[i];
        }
      }
    }
  }
  return String::clone(arena, buf, buflen);
}

template <size_t MaxSize = 8192, typename... Args> void print(const char* fmt, Args&&... args)
{
  using namespace ts::literals;
  StackArena<MaxSize> arena;
  String result = format(arena, fmt, args...);
  __format_output_stdout(result);
  __format_output_stdout("\n"_s);
}

template <size_t MaxSize = 8192, typename... Args> void print_nnl(const char* fmt, Args&&... args)
{
  StackArena<MaxSize> arena;
  String result = format(arena, fmt, args...);
  __format_output_stdout(result);
}

template <size_t MaxSize = 8192, typename... Args> void print_stderr(const char* fmt, Args&&... args)
{
  using namespace ts::literals;
  StackArena<MaxSize> arena;
  String result = format(arena, fmt, args...);
  __format_output_stderr(result);
  __format_output_stderr("\n"_s);
}

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

struct Color;
enum struct ParseError { NonDigitsRemaining };
[[nodiscard]] Result<int64_t, ParseError> strToInt(String string);
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

#ifdef TINYSTD_USE_CLAY
  Color(Clay_Color color)
  {
    this->r = color.r;
    this->g = color.g;
    this->b = color.b;
    this->a = color.a;
  }
#endif

  Color(String color)
  {
    *this = hexToColor(color);
  }

  Color(const char* color)
  {
    *this = hexToColor(String::view(color));
  }

#ifdef TINYSTD_USE_CLAY
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
#endif // USE_CLAY
};

} // namespace ts

// ===========================================================
// ===                    IMPLEMENTATION                   ===
// ===========================================================

#ifdef TINYSTD_IMPLEMENTATION

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace ts {

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
Arena Arena::create(size_t chunkSize)
{
  size_t allocSize = sizeof(ArenaChunk) + chunkSize;
  Arena newArena;
  newArena.firstChunk = (ArenaChunk*)calloc(allocSize, 1);
  newArena.isStackArena = false;
  newArena.__initialized = true;
  if (newArena.firstChunk == 0) {
    __panicSizeT("Arena chunk allocation of size {} failed", allocSize);
  }
  newArena.firstChunk->capacity = chunkSize;
  return newArena;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
Arena Arena::createFromBuffer(char* buffer, size_t bufferSize)
{
  if (bufferSize <= sizeof(ArenaChunk)) {
    __panicStr("Cannot create arena from buffer: Buffer too small");
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

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void Arena::enlarge(ArenaChunk** lastChunk, size_t chunkSize)
{
  print("Warning: Arena was enlarged. Consider more short-lived arenas to prevent excessive memory usage.");
  if (!this->__initialized) {
    __panicStr("Arena was not properly initialized");
  }
  if (this->isStackArena) {
    __panicStr("Cannot enlarge a stack-based arena");
  }
  size_t allocSize = sizeof(ArenaChunk) + chunkSize;
  (*lastChunk)->nextChunk = (ArenaChunk*)calloc(allocSize, 1);
  if ((*lastChunk)->nextChunk == 0) {
    __panicSizeT("Arena chunk allocation of size {} failed", allocSize);
  }
  (*lastChunk) = (*lastChunk)->nextChunk;
  (*lastChunk)->capacity = chunkSize;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void Arena::free()
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

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void Arena::clearAndReinit()
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

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::concat(Arena& arena, String other)
{
  char* buf = arena.allocate<char>(this->length + other.length + 1);
  memcpy(buf, this->data, this->length);
  memcpy(buf + this->length, other.data, other.length);
  return String(buf, this->length + other.length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
size_t String::find(String delimiter, size_t startIndex)
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

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::substr(size_t startIndex, size_t count) const
{
  if (startIndex >= length) {
    return { nullptr, 0 };
  }

  if (count == npos || startIndex + count > length) {
    count = length - startIndex;
  }

  return { data + startIndex, count };
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
char String::get(size_t index)
{
  if (index >= this->length) {
    __panicStr("String index access out of range");
  }
  return this->data[index];
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
char String::operator[](size_t index)
{
  return this->get(index);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
const char* String::c_str(Arena& arena)
{
  String s = String::clone(arena, *this);
  return s.data;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
bool String::startsWith(String other) const
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

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
bool String::endsWith(String other) const
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

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::clone(Arena& arena, String string)
{
  char* buf = arena.allocate<char>(string.length + 1);
  memcpy(buf, string.data, string.length);
  return String(buf, string.length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::clone(Arena& arena, const char* str)
{
  return String::clone(arena, str, strlen(str));
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::clone(Arena& arena, const char* str, size_t length)
{
  char* buf = arena.allocate<char>(length + 1);
  memcpy(buf, str, length);
  return String(buf, length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::view(const char* str)
{
  return String(str, strlen(str));
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::view(const char* str, size_t length)
{
  return String(str, length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
bool String::operator==(String other)
{
  if (this->length != other.length) {
    return false;
  }
  return 0 == memcmp(this->data, other.data, this->length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __panicImpl(String str)
{
  print_stderr("[FATAL] Thread panicked: {}", str);
  fflush(stderr);
  abort();
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __format_vsnprintf(char* buffer, size_t bufferSize, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, bufferSize, format, args);
  va_end(args);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
size_t __format_strlen(const char* str)
{
  return strlen(str);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void* __memset(void* dest, int ch, size_t count)
{
  return memset(dest, ch, count);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __format_output_stdout(String string)
{
  fwrite(string.data, 1, string.length, stdout);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __format_output_stderr(String string)
{
  fwrite(string.data, 1, string.length, stderr);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __panicStr(const char* str)
{
  panic("{}", str);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __panicSizeT(const char* str, size_t value)
{
  panic(str, value);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
Result<int64_t, ParseError> strToInt(String string)
{
  char* endptr;
  StackArena<64> tmpArena;
  int64_t value = strtol(string.c_str(tmpArena), &endptr, 10);
  if (*endptr != '\0') {
    return ParseError::NonDigitsRemaining;
  }
  return value;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
uint8_t hexToDigit(char letter)
{
  if (letter >= '0' && letter <= '9') {
    return letter - '0';
  } else if (letter >= 'a' && letter <= 'f') {
    return letter - 'a' + 10;
  } else if (letter >= 'A' && letter <= 'F') {
    return letter - 'A' + 10;
  } else {
    return 0;
  }
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
Color hexToColor(String hex)
{
  if (hex.length == 4) {
    uint8_t r = hexToDigit(hex[1]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[2]) + 16 * hexToDigit(hex[2]);
    uint8_t b = hexToDigit(hex[3]) + 16 * hexToDigit(hex[3]);
    uint8_t a = 255;
    return Color(r, g, b, a);
  } else if (hex.length == 5) {
    uint8_t r = hexToDigit(hex[1]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[2]) + 16 * hexToDigit(hex[2]);
    uint8_t b = hexToDigit(hex[3]) + 16 * hexToDigit(hex[3]);
    uint8_t a = hexToDigit(hex[4]) + 16 * hexToDigit(hex[4]);
    return Color(r, g, b, a);
  } else if (hex.length == 7) {
    uint8_t r = hexToDigit(hex[2]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[4]) + 16 * hexToDigit(hex[3]);
    uint8_t b = hexToDigit(hex[6]) + 16 * hexToDigit(hex[5]);
    uint8_t a = 255;
    return Color(r, g, b, a);
  } else if (hex.length == 9) {
    uint8_t r = hexToDigit(hex[2]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[4]) + 16 * hexToDigit(hex[3]);
    uint8_t b = hexToDigit(hex[6]) + 16 * hexToDigit(hex[5]);
    uint8_t a = hexToDigit(hex[8]) + 16 * hexToDigit(hex[7]);
    return Color(r, g, b, a);
  } else {
    return Color(0, 0, 0, 0);
  }
}

} // namespace ts

#endif // TINYSTD_IMPLEMENTATION

#endif // TINYSTD_H
