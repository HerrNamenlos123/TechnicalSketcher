
#include "shared.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"

#if defined(TSK_LINUX)
#include "platform/linux-platform.cpp"
#elif defined(TSK_WINDOWS)
#include "platform/win32-platform.cpp"
#else
#error "Unsupported platform/Platform not set by build system"
#endif
