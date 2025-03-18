
struct String;
struct Arena;

template <typename... Args>
String format(Arena* arena, const char* fmt, Args&&... args);