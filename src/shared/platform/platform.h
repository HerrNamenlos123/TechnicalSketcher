
#ifndef TSK_PLATFORM_H
#define TSK_PLATFORM_H

#include "../std.h"
#include <errno.h>

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

[[nodiscard]] extern String PlatformFSErrorToString(Arena& arena, SystemError);

[[nodiscard]] extern Result<time_t, SystemError> GetFileModificationDate(Arena& arena, String path);

[[nodiscard]] extern Result<List<String>, SystemError> ListDirectory(Arena& arena, String path);

#endif // TSK_PLATFORM_H