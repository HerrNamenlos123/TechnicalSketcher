
#include <windows.h>

#include "../format.h"
#include "platform.h"
#include <errhandlingapi.h>
#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <time.h>

#include "platform.h"

#include <direct.h>

#undef LoadLibrary

String utf16_to_utf8(Arena& arena, const wchar_t* wstr)
{
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
  char* buf = arena.allocate<char>(size_needed);
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buf, size_needed, nullptr, nullptr);
  return String::view(buf, size_needed - 1);
}

wchar_t* utf8_to_utf16(Arena& arena, String str)
{
  auto c_str = str.c_str(arena);
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, c_str, -1, nullptr, 0);
  wchar_t* buf = arena.allocate<wchar_t>(size_needed);
  MultiByteToWideChar(CP_UTF8, 0, c_str, -1, buf, size_needed);
  return buf;
}

Result<String, SystemError> GetCurrentWorkingDirectory(Arena& arena)
{
  wchar_t path[MAX_PATH];
  auto result = GetCurrentDirectoryW(MAX_PATH, path);
  if (result == 0) {
    return (SystemError)errno;
  }
  return String::clone(arena, utf16_to_utf8(arena, path));
}

Result<bool, SystemError> IsRegularFile(Arena& arena, String path)
{
  DWORD attributes = GetFileAttributesW(utf8_to_utf16(arena, path));
  if (attributes == INVALID_FILE_ATTRIBUTES) {
    return (SystemError)GetLastError();
  }
  return (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

extern String PlatformFSErrorToString(Arena& arena, SystemError error)
{
  wchar_t* message_buffer = nullptr;
  FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      (DWORD)error,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPWSTR)&message_buffer,
      0,
      nullptr);

  String result = utf16_to_utf8(arena, message_buffer);
  LocalFree(message_buffer);
  return result;
}

Result<time_t, SystemError> GetFileModificationDate(String path)
{
  if (path.length > MAX_PATH) {
    return SystemError::NameTooLong;
  }
  struct stat fileStat;
  StackArena<MAX_PATH + 1> arena;
  if (stat(path.c_str(arena), &fileStat) != 0) {
    return (SystemError)errno;
  }
  return fileStat.st_mtime;
}

Result<List<String>, SystemError> ListDirectory(Arena& arena, String path)
{
  List<String> list;

  // Prepare the path for FindFirstFile (include a wildcard to match all files)
  wchar_t* search_path = utf8_to_utf16(arena, path.concat(arena, "\\*"_s));

  WIN32_FIND_DATAW findFileData;
  HANDLE hFind = FindFirstFileW(search_path, &findFileData);

  if (hFind == INVALID_HANDLE_VALUE) {
    return (SystemError)GetLastError(); // Return Windows error code
  }

  do {
    // Skip "." and ".."
    if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
      String relative = format(arena, "{}\\{}", path, utf16_to_utf8(arena, findFileData.cFileName));
      list.push(arena, relative);
    }
  } while (FindNextFileW(hFind, &findFileData) != 0);

  DWORD dwError = GetLastError();
  if (dwError != ERROR_NO_MORE_FILES) {
    FindClose(hFind);
    return (SystemError)dwError;
  }

  FindClose(hFind);
  return list;
}

Result<void*, String> LoadLibrary(Arena& arena, String path)
{
  auto handle = LoadLibraryW(utf8_to_utf16(arena, path));
  if (!handle) {
    auto error = GetLastError();
    return PlatformFSErrorToString(arena, (SystemError)error);
  }
  return handle;
}

Result<void*, String> LoadLibraryFunc(Arena& arena, void* library, String funcname)
{
  auto func = (void*)GetProcAddress((HMODULE)library, funcname.c_str(arena));
  if (!func) {
    auto error = GetLastError();
    return PlatformFSErrorToString(arena, (SystemError)error);
  }
  return func;
}

void UnloadLibrary(void* handle)
{
  FreeLibrary((HMODULE)handle);
}