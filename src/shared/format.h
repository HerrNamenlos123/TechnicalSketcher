
#ifndef FORMAT_H
#define FORMAT_H

#include "std.h"
#include "stddecl.h"

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

template <typename T> size_t format_value(T value, String formatArg, char* buffer, size_t remainingBufferSize)
{
  if constexpr (is_same_v<T, int>) {
    __format_vsnprintf(buffer, remainingBufferSize, "%d", value);
    return __format_strlen(buffer);
  } else if constexpr (is_same_v<T, double>) {
    __format_vsnprintf(buffer, remainingBufferSize, "%lf", value);
    return __format_strlen(buffer);
  } else if constexpr (is_same_v<T, unsigned long>) {
    __format_vsnprintf(buffer, remainingBufferSize, "%lu", value);
    return __format_strlen(buffer);
  } else if constexpr (is_same_v<T, float>) {
    __format_vsnprintf(buffer, remainingBufferSize, "%f", value);
    return __format_strlen(buffer);
  } else if constexpr (is_same_v<T, String>) {
    for (size_t i = 0; i < value.length; i++) {
      if (i < remainingBufferSize) {
        buffer[i] = value.data[i];
      }
    }
    return value.length;
  } else if constexpr (is_same_v<T, const char*>) {
    __format_vsnprintf(buffer, remainingBufferSize, "%s", value);
    return __format_strlen(buffer);
  } else if constexpr (is_same_v<T, char*>) {
    __format_vsnprintf(buffer, remainingBufferSize, "%s", value);
    return __format_strlen(buffer);
  } else {
    static_assert(false, "No formatter available for this type");
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
  StackArena<MaxSize> arena;
  String result = format(arena, fmt, args...);
  __format_output_stderr(result);
  __format_output_stderr("\n"_s);
}

#endif // FORMAT_H