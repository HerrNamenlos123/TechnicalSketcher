
#include "platform.h"
#include <cerrno>
#include <cstddef>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "platform.h"

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
#error It seems like the XSI version of strerror_r is provided, but we use the GNU version
#endif

Result<String, SystemError> GetCurrentWorkingDirectory(Arena& arena)
{
  char path[PATH_MAX];
  auto result = getcwd(path, sizeof(path));
  if (result == 0) {
    return (SystemError)errno;
  }
  return String::clone(arena, path);
}

Result<bool, SystemError> IsRegularFile(Arena& arena, String path)
{
  struct stat fileStat;
  if (stat(path.c_str(arena), &fileStat) != 0) {
    return (SystemError)errno;
  }
  return S_ISREG(fileStat.st_mode);
}

extern String PlatformFSErrorToString(Arena& arena, SystemError error)
{
  char buf[4096];
  char* result = strerror_r((int)error, buf, sizeof(buf));
  if (result == 0) {
    if (*result == EINVAL) {
      return String::view("[The value of errnum is not a valid error number]");
    } else if (*result == ERANGE) {
      return String::view("[Insufficient storage was supplied to contain the error description string]");
    } else {
      return String::view("[Unknown error while stringifying errno]");
    }
  }
  return String::clone(arena, result);
}

Result<time_t, SystemError> GetFileModificationDate(String path)
{
  if (path.length > PATH_MAX) {
    return SystemError::NameTooLong;
  }
  struct stat fileStat;
  StackArena<PATH_MAX + 1> arena;
  if (stat(path.c_str(arena), &fileStat) != 0) {
    return (SystemError)errno;
  }
  return fileStat.st_mtime;
}

Result<List<String>, SystemError> ListDirectory(Arena& arena, String path)
{
  List<String> list;

  struct dirent* entry;
  DIR* dp = opendir(path.c_str(arena));
  if (dp == NULL) {
    return (SystemError)errno;
  }

  while ((entry = readdir(dp)) != NULL) {
    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
      auto relative = format(arena, "{}/{}", path, entry->d_name);
      char absolute[PATH_MAX];
      if (realpath(relative.c_str(arena), absolute) == 0) {
        return (SystemError)errno;
      } else {
        list.push(arena, String::clone(arena, absolute));
      }
    }
  }

  closedir(dp);
  return list;
}