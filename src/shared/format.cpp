
#include "app.h"
#include "platform/platform.h"
#include "std.h"
#include <fmt/core.h>
#include <stdarg.h>

template <>
struct fmt::formatter<String> {
  constexpr auto parse(fmt::format_parse_context& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const String& s, FormatContext& ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", std::string(s.data, s.length));
  }
};

template <>
struct fmt::formatter<SystemError> {
  constexpr auto parse(fmt::format_parse_context& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const SystemError& error, FormatContext& ctx) const
  {
    StackArena<8192> arena;
    return fmt::format_to(ctx.out(), "{}", PlatformFSErrorToString(arena, error));
  }
};

template <typename... Args>
String format(Arena& arena, const char* fmt, Args&&... args)
{
  try {
    std::string result = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    return String::clone(arena, result.c_str(), result.length());
  } catch (const std::exception& e) {
    std::string result = fmt::format("{{{}}}", e.what());
    return String::clone(arena, result.c_str(), result.length());
  }
}

template <typename... Args>
void print(const char* fmt, Args&&... args)
{
  std::string result;
  try {
    result = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
  } catch (const std::exception& e) {
    result = fmt::format("{{{}}}", e.what());
  }
  printf("%s\n", result.c_str());
}

template <typename... Args>
void print_stderr(const char* fmt, Args&&... args)
{
  std::string result;
  try {
    result = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
  } catch (const std::exception& e) {
    result = fmt::format("{{{}}}", e.what());
  }
  printf("%s\n", result.c_str());
}

#define FORMAT_IMPL_0()                              \
  template String format(Arena& arena, const char*); \
  template void print(const char*);                  \
  template void print_stderr(const char*);

#define FORMAT_IMPL_1(T)                                  \
  template String format(Arena& arena, const char*, T&);  \
  template void print(const char*, T&);                   \
  template void print_stderr(const char*, T&);            \
  template String format(Arena& arena, const char*, T&&); \
  template void print(const char*, T&&);                  \
  template void print_stderr(const char*, T&&);

#define FORMAT_IMPL_2(T, U)                                    \
  template String format(Arena& arena, const char*, T&, U&);   \
  template void print(const char*, T&, U&);                    \
  template void print_stderr(const char*, T&, U&);             \
  template String format(Arena& arena, const char*, T&&, U&&); \
  template void print(const char*, T&&, U&&);                  \
  template void print_stderr(const char*, T&&, U&&);

#define FORMAT_IMPL_3(T, U, V)                                   \
  template String format(Arena& arena, const char*, T&, U&, V&); \
  template void print(const char*, T&, U&, V&);                  \
  template void print_stderr(const char*, T&, U&, V&);

#define FORMAT_IMPL_4(T, U, V, W)                                    \
  template String format(Arena& arena, const char*, T&, U&, V&, W&); \
  template void print(const char*, T&, U&, V&, W&);                  \
  template void print_stderr(const char*, T&, U&, V&, W&);

FORMAT_IMPL_0()
FORMAT_IMPL_1(SystemError)
FORMAT_IMPL_1(char*)
FORMAT_IMPL_1(const char*)
FORMAT_IMPL_1(int)
FORMAT_IMPL_1(float)
FORMAT_IMPL_1(double)
FORMAT_IMPL_1(time_t)
FORMAT_IMPL_1(size_t)
FORMAT_IMPL_1(char)
FORMAT_IMPL_1(String)
FORMAT_IMPL_2(int, int)
FORMAT_IMPL_2(float, float)
FORMAT_IMPL_2(double, double)
FORMAT_IMPL_2(size_t, size_t)
FORMAT_IMPL_2(int, double)
FORMAT_IMPL_2(double, int)
FORMAT_IMPL_2(int, float)
FORMAT_IMPL_2(float, int)
FORMAT_IMPL_2(double, float)
FORMAT_IMPL_2(float, double)
FORMAT_IMPL_2(String, String)
FORMAT_IMPL_2(String, int)
FORMAT_IMPL_2(String, float)
FORMAT_IMPL_2(String, double)
FORMAT_IMPL_2(float, String)
FORMAT_IMPL_2(int, String)
FORMAT_IMPL_2(double, String)