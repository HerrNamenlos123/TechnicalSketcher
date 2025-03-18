
#include "platform.h"
#include <sys/stat.h>
#include <time.h>

time_t GetFileModificationDate(Arena* arena, String path)
{
  struct stat fileStat;
  if (stat(c_str(arena, path), &fileStat) == 0) {
    return fileStat.st_mtime;
  }
  return 0;
}