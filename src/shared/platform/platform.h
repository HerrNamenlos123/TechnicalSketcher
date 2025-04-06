
#ifndef TSK_PLATFORM_H
#define TSK_PLATFORM_H

#include "../TinyStd.hpp"
#include <errno.h>
#include <time.h>

using ts::Arena;
using ts::List;
using ts::Result;
using ts::StackArena;
using ts::String;

enum struct SystemError {
  PermissionDenied = EACCES,
  BadFileDescriptor = EBADF,
  ProcessFileLimitReached = EMFILE,
  SystemFileLimitReached = ENFILE,
  NoSuchFileOrDirectory = ENOENT,
  OutOfMemory = ENOMEM,
  NotADirectory = ENOTDIR,
  BadAddress = EFAULT,
  InvalidFlag = EINVAL,
  LoopSymbolicLinks = ELOOP,
  NameTooLong = ENAMETOOLONG,
  FileIdOverflow = EOVERFLOW,
  IOError = EIO,
  InvalidRange = ERANGE,
};

[[nodiscard]] extern Result<String, SystemError> GetCurrentWorkingDirectory(Arena& arena);

[[nodiscard]] extern Result<bool, SystemError> IsRegularFile(ts::Arena& arena, String path);

[[nodiscard]] extern String PlatformFSErrorToString(Arena& arena, SystemError);

[[nodiscard]] extern Result<time_t, SystemError> GetFileModificationDate(String path);

[[nodiscard]] extern Result<List<String>, SystemError> ListDirectory(Arena& arena, String path);

[[nodiscard]] extern Result<void*, String> LoadLibrary(Arena& arena, String path);

[[nodiscard]] extern Result<void*, String> LoadLibraryFunc(Arena& arena, void* library, String funcname);

extern void UnloadLibrary(void* handle);

#endif // TSK_PLATFORM_H