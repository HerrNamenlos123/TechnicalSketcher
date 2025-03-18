
#include "../base.h"

#include "platform/platform.h"

// String format(Arena* arena, const char* fmt, ...);
template <typename... Args>
String format(Arena* arena, const char* fmt, Args&&... args);