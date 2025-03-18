
#include "app.h"
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

template <typename... Args>
String format(Arena* arena, const char* fmt, Args&&... args)
{
  try {
    std::string result = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    return String::allocate(arena, result.c_str(), result.length());
  } catch (const std::exception& e) {
    return String("{").concat(arena, String(e.what())).concat(arena, String("}"));
  }
}

#define FORMAT_IMPL_1(T) template String format<T>(Arena * arena, const char*, T&&);
#define FORMAT_IMPL_2(T, U) template String format<T, U>(Arena * arena, const char*, T&&, U&&);
#define FORMAT_IMPL_3(T, U, V) template String format<T, U>(Arena * arena, const char*, T&&, U&&, V&&);
#define FORMAT_IMPL_4(T, U, V, W) template String format<T, U>(Arena * arena, const char*, T&&, U&&, V&&, W&&);

FORMAT_IMPL_1(int)
FORMAT_IMPL_1(float)
FORMAT_IMPL_1(double)
FORMAT_IMPL_1(char)
FORMAT_IMPL_1(char*)
FORMAT_IMPL_1(String)
FORMAT_IMPL_2(int, int)
FORMAT_IMPL_2(float, float)
FORMAT_IMPL_2(double, double)
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