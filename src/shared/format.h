
#ifndef FORMAT_H
#define FORMAT_H

#include "std.h"

// Formatting:
// {} - default formatting
// The format_impl() function is called twice for formatting each argument.
// On the first call, target is null and you must return the length of the formatted string.
// Then, a buffer is allocated that is just large enough. Finally, format_impl() is called again with a buffer.

static inline size_t format_value(int value, String formatString, Optional<char*> buffer, size_t remainingBufferSize)
{
  if (buffer) {
    __format_vsnprintf(buffer.value(), remainingBufferSize, "%d", value);
    return __format_strlen(buffer.value());
  } else {
    char buffer[21] = { 0 };
    __format_vsnprintf(buffer, sizeof(buffer) - 1, "%d", value);
    return __format_strlen(buffer);
  }
}

template <typename... Args>
String format_impl(Arena& arena, const char* fmt)
{
}

template <typename T, typename... Args>
String format_impl(Arena& arena, const char* fmt, Optional<char*> buffer, T&& value, Args&&... args)
{
  if (buffer) {

  } else {
    size_t requiredSize = format_value(value, "", {}, 0);
    return format_impl(arena, fmt, args...);
  }
}

#include <stdio.h>

template <typename... Args>
String format(Arena& arena, const char* fmt, Args&&... args)
{
  // I don't want to use dynamic allocations to keep the main arena clean,
  // hence just use a fixed size buffer and limit the maximum number of arguments.
  Array<char[20], 20> formatArgsBuf;
  Array<String, 20> formatArgs;
  size_t numFormatArgs = 0;

  bool isParsingArg = false;
  size_t fmtLen = __format_strlen(fmt);
  for (size_t i = 0; i < fmtLen; i++) {
    if (!isParsingArg) {
      if (fmt[i] == '{') {
        isParsingArg = true;
        formatArgs[numFormatArgs] = String::view(formatArgsBuf[numFormatArgs], 0);
      }
    } else {
      if (fmt[i] == '{') {
        isParsingArg = false;
      } else if (fmt[i] == '}') {
        isParsingArg = false;
        numFormatArgs++;
      } else {
        if (formatArgs[numFormatArgs].length < sizeof(formatArgsBuf[numFormatArgs])) {
          formatArgsBuf[numFormatArgs][formatArgs[numFormatArgs].length] = fmt[i];
          formatArgs[numFormatArgs].length++;
        }
      }
    }
  }

  printf("Formatted: '%s'\n", fmt);
  for (size_t i = 0; i < numFormatArgs; i++) {
    printf("Arg %zu: '%s'\n", i, formatArgs[i].c_str(arena));
  }

  //   format_impl(arena, 50, "");
  // std::string result = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
  //   return String::clone(arena, result.c_str(), result.length());
  return "[formatted]"_s;
}

template <typename... Args>
void print(const char* fmt, Args&&... args)
{
  //   std::string result;
  //   try {
  //     result = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
  //   } catch (const std::exception& e) {
  //     result = fmt::format("{{{}}}", e.what());
  //   }
  //   printf("%s\n", result.c_str());
}

template <typename... Args>
void print_stderr(const char* fmt, Args&&... args)
{
  //   std::string result;
  //   try {
  //     result = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
  //   } catch (const std::exception& e) {
  //     result = fmt::format("{{{}}}", e.what());
  //   }
  fprintf(stderr, "%s\n", fmt);
}

#endif // FORMAT_H