
#ifndef TINYSTD_H
#define TINYSTD_H

#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>

#if defined(TINYSTD_USE_CLAY) && !defined(CLAY_HEADER)
#error TINYSTD_USE_CLAY is defined, but CLAY_HEADER not. This means that you want to use the Clay library bindings, but clay.h was not yet included. You must manually include clay.h BEFORE TinyStd.hpp.
#endif

// ======================================================================
//  *                 BEGIN Subprocess.h Library                       *
// ======================================================================

/*
   The latest version of this library is available on GitHub;
   https://github.com/sheredom/subprocess.h
*/

/*
   This is free and unencumbered software released into the public domain.

   Anyone is free to copy, modify, publish, use, compile, sell, or
   distribute this software, either in source code form or as a compiled
   binary, for any purpose, commercial or non-commercial, and by any
   means.

   In jurisdictions that recognize copyright laws, the author or authors
   of this software dedicate any and all copyright interest in the
   software to the public domain. We make this dedication for the benefit
   of the public at large and to the detriment of our heirs and
   successors. We intend this dedication to be an overt act of
   relinquishment in perpetuity of all present and future rights to this
   software under copyright law.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.

   For more information, please refer to <http://unlicense.org/>
*/

#ifndef SHEREDOM_SUBPROCESS_H_INCLUDED
#define SHEREDOM_SUBPROCESS_H_INCLUDED

#if defined(_MSC_VER)
#pragma warning(push, 1)

/* disable warning: '__cplusplus' is not defined as a preprocessor macro,
 * replacing with '0' for '#if/#elif' */
#pragma warning(disable : 4668)
#endif

#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(__TINYC__)
#define SUBPROCESS_ATTRIBUTE(a) __attribute((a))
#else
#define SUBPROCESS_ATTRIBUTE(a) __attribute__((a))
#endif

#if defined(_MSC_VER)
#define subprocess_pure
#define subprocess_weak __inline
#define subprocess_tls __declspec(thread)
#elif defined(__MINGW32__)
#define subprocess_pure SUBPROCESS_ATTRIBUTE(pure)
#define subprocess_weak static SUBPROCESS_ATTRIBUTE(used)
#define subprocess_tls __thread
#elif defined(__clang__) || defined(__GNUC__) || defined(__TINYC__)
#define subprocess_pure SUBPROCESS_ATTRIBUTE(pure)
#define subprocess_weak SUBPROCESS_ATTRIBUTE(weak)
#define subprocess_tls __thread
#else
#error Non clang, non gcc, non MSVC compiler found!
#endif

struct subprocess_s;

enum subprocess_option_e {
  // stdout and stderr are the same FILE.
  subprocess_option_combined_stdout_stderr = 0x1,

  // The child process should inherit the environment variables of the parent.
  subprocess_option_inherit_environment = 0x2,

  // Enable asynchronous reading of stdout/stderr before it has completed.
  subprocess_option_enable_async = 0x4,

  // Enable the child process to be spawned with no window visible if supported
  // by the platform.
  subprocess_option_no_window = 0x8,

  // Search for program names in the PATH variable. Always enabled on Windows.
  // Note: this will **not** search for paths in any provided custom environment
  // and instead uses the PATH of the spawning process.
  subprocess_option_search_user_path = 0x10
};

#if defined(__cplusplus)
extern "C" {
#endif

/// @brief Create a process.
/// @param command_line An array of strings for the command line to execute for
/// this process. The last element must be NULL to signify the end of the array.
/// The memory backing this parameter only needs to persist until this function
/// returns.
/// @param options A bit field of subprocess_option_e's to pass.
/// @param out_process The newly created process.
/// @return On success zero is returned.
subprocess_weak int subprocess_create(
    const char* const command_line[], int options, struct subprocess_s* const out_process);

/// @brief Create a process (extended create).
/// @param command_line An array of strings for the command line to execute for
/// this process. The last element must be NULL to signify the end of the array.
/// The memory backing this parameter only needs to persist until this function
/// returns.
/// @param options A bit field of subprocess_option_e's to pass.
/// @param environment An optional array of strings for the environment to use
/// for a child process (each element of the form FOO=BAR). The last element
/// must be NULL to signify the end of the array.
/// @param out_process The newly created process.
/// @return On success zero is returned.
///
/// If `options` contains `subprocess_option_inherit_environment`, then
/// `environment` must be NULL.
subprocess_weak int subprocess_create_ex(const char* const command_line[], int options, const char* const environment[],
    struct subprocess_s* const out_process);

/// @brief Get the standard input file for a process.
/// @param process The process to query.
/// @return The file for standard input of the process.
///
/// The file returned can be written to by the parent process to feed data to
/// the standard input of the process.
subprocess_pure subprocess_weak FILE* subprocess_stdin(const struct subprocess_s* const process);

/// @brief Get the standard output file for a process.
/// @param process The process to query.
/// @return The file for standard output of the process.
///
/// The file returned can be read from by the parent process to read data from
/// the standard output of the child process.
subprocess_pure subprocess_weak FILE* subprocess_stdout(const struct subprocess_s* const process);

/// @brief Get the standard error file for a process.
/// @param process The process to query.
/// @return The file for standard error of the process.
///
/// The file returned can be read from by the parent process to read data from
/// the standard error of the child process.
///
/// If the process was created with the subprocess_option_combined_stdout_stderr
/// option bit set, this function will return NULL, and the subprocess_stdout
/// function should be used for both the standard output and error combined.
subprocess_pure subprocess_weak FILE* subprocess_stderr(const struct subprocess_s* const process);

/// @brief Wait for a process to finish execution.
/// @param process The process to wait for.
/// @param out_return_code The return code of the returned process (can be
/// NULL).
/// @return On success zero is returned.
///
/// Joining a process will close the stdin pipe to the process.
subprocess_weak int subprocess_join(struct subprocess_s* const process, int* const out_return_code);

/// @brief Destroy a previously created process.
/// @param process The process to destroy.
/// @return On success zero is returned.
///
/// If the process to be destroyed had not finished execution, it may out live
/// the parent process.
subprocess_weak int subprocess_destroy(struct subprocess_s* const process);

/// @brief Terminate a previously created process.
/// @param process The process to terminate.
/// @return On success zero is returned.
///
/// If the process to be destroyed had not finished execution, it will be
/// terminated (i.e killed).
subprocess_weak int subprocess_terminate(struct subprocess_s* const process);

/// @brief Read the standard output from the child process.
/// @param process The process to read from.
/// @param buffer The buffer to read into.
/// @param size The maximum number of bytes to read.
/// @return The number of bytes actually read into buffer. Can only be 0 if the
/// process has complete.
///
/// The only safe way to read from the standard output of a process during it's
/// execution is to use the `subprocess_option_enable_async` option in
/// conjunction with this method.
subprocess_weak unsigned subprocess_read_stdout(struct subprocess_s* const process, char* const buffer, unsigned size);

/// @brief Read the standard error from the child process.
/// @param process The process to read from.
/// @param buffer The buffer to read into.
/// @param size The maximum number of bytes to read.
/// @return The number of bytes actually read into buffer. Can only be 0 if the
/// process has complete.
///
/// The only safe way to read from the standard error of a process during it's
/// execution is to use the `subprocess_option_enable_async` option in
/// conjunction with this method.
subprocess_weak unsigned subprocess_read_stderr(struct subprocess_s* const process, char* const buffer, unsigned size);

/// @brief Returns if the subprocess is currently still alive and executing.
/// @param process The process to check.
/// @return If the process is still alive non-zero is returned.
subprocess_weak int subprocess_alive(struct subprocess_s* const process);

#if defined(__cplusplus)
#define SUBPROCESS_CAST(type, x) static_cast<type>(x)
#define SUBPROCESS_PTR_CAST(type, x) reinterpret_cast<type>(x)
#define SUBPROCESS_CONST_CAST(type, x) const_cast<type>(x)
#define SUBPROCESS_NULL NULL
#else
#define SUBPROCESS_CAST(type, x) ((type)(x))
#define SUBPROCESS_PTR_CAST(type, x) ((type)(x))
#define SUBPROCESS_CONST_CAST(type, x) ((type)(x))
#define SUBPROCESS_NULL 0
#endif

#if !defined(_WIN32)
#include <signal.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#if defined(_WIN32)

#if (_MSC_VER < 1920)
#ifdef _WIN64
typedef __int64 subprocess_intptr_t;
typedef unsigned __int64 subprocess_size_t;
#else
typedef int subprocess_intptr_t;
typedef unsigned int subprocess_size_t;
#endif
#else
#include <inttypes.h>

typedef intptr_t subprocess_intptr_t;
typedef size_t subprocess_size_t;
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif

typedef struct _PROCESS_INFORMATION* LPPROCESS_INFORMATION;
typedef struct _SECURITY_ATTRIBUTES* LPSECURITY_ATTRIBUTES;
typedef struct _STARTUPINFOA* LPSTARTUPINFOA;
typedef struct _OVERLAPPED* LPOVERLAPPED;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif
#ifdef __MINGW32__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

struct subprocess_subprocess_information_s {
  void* hProcess;
  void* hThread;
  unsigned long dwProcessId;
  unsigned long dwThreadId;
};

struct subprocess_security_attributes_s {
  unsigned long nLength;
  void* lpSecurityDescriptor;
  int bInheritHandle;
};

struct subprocess_startup_info_s {
  unsigned long cb;
  char* lpReserved;
  char* lpDesktop;
  char* lpTitle;
  unsigned long dwX;
  unsigned long dwY;
  unsigned long dwXSize;
  unsigned long dwYSize;
  unsigned long dwXCountChars;
  unsigned long dwYCountChars;
  unsigned long dwFillAttribute;
  unsigned long dwFlags;
  unsigned short wShowWindow;
  unsigned short cbReserved2;
  unsigned char* lpReserved2;
  void* hStdInput;
  void* hStdOutput;
  void* hStdError;
};

struct subprocess_overlapped_s {
  uintptr_t Internal;
  uintptr_t InternalHigh;
  union {
    struct {
      unsigned long Offset;
      unsigned long OffsetHigh;
    } DUMMYSTRUCTNAME;
    void* Pointer;
  } DUMMYUNIONNAME;

  void* hEvent;
};

#ifdef __MINGW32__
#pragma GCC diagnostic pop
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

__declspec(dllimport) unsigned long __stdcall GetLastError(void);
__declspec(dllimport) int __stdcall SetHandleInformation(void*, unsigned long, unsigned long);
__declspec(dllimport) int __stdcall CreatePipe(void**, void**, LPSECURITY_ATTRIBUTES, unsigned long);
__declspec(dllimport) void* __stdcall CreateNamedPipeA(const char*, unsigned long, unsigned long, unsigned long,
    unsigned long, unsigned long, unsigned long, LPSECURITY_ATTRIBUTES);
__declspec(dllimport) int __stdcall ReadFile(void*, void*, unsigned long, unsigned long*, LPOVERLAPPED);
__declspec(dllimport) unsigned long __stdcall GetCurrentProcessId(void);
__declspec(dllimport) unsigned long __stdcall GetCurrentThreadId(void);
__declspec(dllimport) void* __stdcall CreateFileA(
    const char*, unsigned long, unsigned long, LPSECURITY_ATTRIBUTES, unsigned long, unsigned long, void*);
__declspec(dllimport) void* __stdcall CreateEventA(LPSECURITY_ATTRIBUTES, int, int, const char*);
__declspec(dllimport) int __stdcall CreateProcessA(const char*, char*, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
    int, unsigned long, void*, const char*, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
__declspec(dllimport) int __stdcall CloseHandle(void*);
__declspec(dllimport) unsigned long __stdcall WaitForSingleObject(void*, unsigned long);
__declspec(dllimport) int __stdcall GetExitCodeProcess(void*, unsigned long* lpExitCode);
__declspec(dllimport) int __stdcall TerminateProcess(void*, unsigned int);
__declspec(dllimport) unsigned long __stdcall WaitForMultipleObjects(unsigned long, void* const*, int, unsigned long);
__declspec(dllimport) int __stdcall GetOverlappedResult(void*, LPOVERLAPPED, unsigned long*, int);

#if defined(_DLL)
#define SUBPROCESS_DLLIMPORT __declspec(dllimport)
#else
#define SUBPROCESS_DLLIMPORT
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif

SUBPROCESS_DLLIMPORT int __cdecl _fileno(FILE*);
SUBPROCESS_DLLIMPORT int __cdecl _open_osfhandle(subprocess_intptr_t, int);
SUBPROCESS_DLLIMPORT subprocess_intptr_t __cdecl _get_osfhandle(int);

#ifndef __MINGW32__
void* __cdecl _alloca(subprocess_size_t);
#else
#include <malloc.h>
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#else
typedef size_t subprocess_size_t;
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif
struct subprocess_s {
  FILE* stdin_file;
  FILE* stdout_file;
  FILE* stderr_file;

#if defined(_WIN32)
  void* hProcess;
  void* hStdInput;
  void* hEventOutput;
  void* hEventError;
#else
  pid_t child;
  int return_status;
#endif

  subprocess_size_t alive;
};
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if defined(__clang__)
#if __has_warning("-Wunsafe-buffer-usage")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#endif

#if defined(_WIN32)
subprocess_weak int subprocess_create_named_pipe_helper(void** rd, void** wr);
int subprocess_create_named_pipe_helper(void** rd, void** wr)
{
  const unsigned long pipeAccessInbound = 0x00000001;
  const unsigned long fileFlagOverlapped = 0x40000000;
  const unsigned long pipeTypeByte = 0x00000000;
  const unsigned long pipeWait = 0x00000000;
  const unsigned long genericWrite = 0x40000000;
  const unsigned long openExisting = 3;
  const unsigned long fileAttributeNormal = 0x00000080;
  const void* const invalidHandleValue = SUBPROCESS_PTR_CAST(void*, ~(SUBPROCESS_CAST(subprocess_intptr_t, 0)));
  struct subprocess_security_attributes_s saAttr = { sizeof(saAttr), SUBPROCESS_NULL, 1 };
  char name[256] = { 0 };
  static subprocess_tls long index = 0;
  const long unique = index++;

#if defined(_MSC_VER) && _MSC_VER < 1900
#pragma warning(push, 1)
#pragma warning(disable : 4996)
  _snprintf(name, sizeof(name) - 1, "\\\\.\\pipe\\sheredom_subprocess_h.%08lx.%08lx.%ld", GetCurrentProcessId(),
      GetCurrentThreadId(), unique);
#pragma warning(pop)
#else
  snprintf(name, sizeof(name) - 1, "\\\\.\\pipe\\sheredom_subprocess_h.%08lx.%08lx.%ld", GetCurrentProcessId(),
      GetCurrentThreadId(), unique);
#endif

  *rd = CreateNamedPipeA(name, pipeAccessInbound | fileFlagOverlapped, pipeTypeByte | pipeWait, 1, 4096, 4096,
      SUBPROCESS_NULL, SUBPROCESS_PTR_CAST(LPSECURITY_ATTRIBUTES, &saAttr));

  if (invalidHandleValue == *rd) {
    return -1;
  }

  *wr = CreateFileA(name, genericWrite, SUBPROCESS_NULL, SUBPROCESS_PTR_CAST(LPSECURITY_ATTRIBUTES, &saAttr),
      openExisting, fileAttributeNormal, SUBPROCESS_NULL);

  if (invalidHandleValue == *wr) {
    return -1;
  }

  return 0;
}
#endif

int subprocess_create(const char* const commandLine[], int options, struct subprocess_s* const out_process)
{
  return subprocess_create_ex(commandLine, options, SUBPROCESS_NULL, out_process);
}

int subprocess_create_ex(const char* const commandLine[], int options, const char* const environment[],
    struct subprocess_s* const out_process)
{
#if defined(_WIN32)
  int fd;
  void *rd, *wr;
  char* commandLineCombined;
  subprocess_size_t len;
  int i, j;
  int need_quoting;
  unsigned long flags = 0;
  const unsigned long startFUseStdHandles = 0x00000100;
  const unsigned long handleFlagInherit = 0x00000001;
  const unsigned long createNoWindow = 0x08000000;
  struct subprocess_subprocess_information_s processInfo;
  struct subprocess_security_attributes_s saAttr = { sizeof(saAttr), SUBPROCESS_NULL, 1 };
  char* used_environment = SUBPROCESS_NULL;
  struct subprocess_startup_info_s startInfo = { 0, SUBPROCESS_NULL, SUBPROCESS_NULL, SUBPROCESS_NULL, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, SUBPROCESS_NULL, SUBPROCESS_NULL, SUBPROCESS_NULL, SUBPROCESS_NULL };

  startInfo.cb = sizeof(startInfo);
  startInfo.dwFlags = startFUseStdHandles;

  if (subprocess_option_no_window == (options & subprocess_option_no_window)) {
    flags |= createNoWindow;
  }

  if (subprocess_option_inherit_environment != (options & subprocess_option_inherit_environment)) {
    if (SUBPROCESS_NULL == environment) {
      used_environment = SUBPROCESS_CONST_CAST(char*, "\0\0");
    } else {
      // We always end with two null terminators.
      len = 2;

      for (i = 0; environment[i]; i++) {
        for (j = 0; '\0' != environment[i][j]; j++) {
          len++;
        }

        // For the null terminator too.
        len++;
      }

      used_environment = SUBPROCESS_CAST(char*, _alloca(len));

      // Re-use len for the insertion position
      len = 0;

      for (i = 0; environment[i]; i++) {
        for (j = 0; '\0' != environment[i][j]; j++) {
          used_environment[len++] = environment[i][j];
        }

        used_environment[len++] = '\0';
      }

      // End with the two null terminators.
      used_environment[len++] = '\0';
      used_environment[len++] = '\0';
    }
  } else {
    if (SUBPROCESS_NULL != environment) {
      return -1;
    }
  }

  if (!CreatePipe(&rd, &wr, SUBPROCESS_PTR_CAST(LPSECURITY_ATTRIBUTES, &saAttr), 0)) {
    return -1;
  }

  if (!SetHandleInformation(wr, handleFlagInherit, 0)) {
    return -1;
  }

  fd = _open_osfhandle(SUBPROCESS_PTR_CAST(subprocess_intptr_t, wr), 0);

  if (-1 != fd) {
    out_process->stdin_file = _fdopen(fd, "wb");

    if (SUBPROCESS_NULL == out_process->stdin_file) {
      return -1;
    }
  }

  startInfo.hStdInput = rd;

  if (options & subprocess_option_enable_async) {
    if (subprocess_create_named_pipe_helper(&rd, &wr)) {
      return -1;
    }
  } else {
    if (!CreatePipe(&rd, &wr, SUBPROCESS_PTR_CAST(LPSECURITY_ATTRIBUTES, &saAttr), 0)) {
      return -1;
    }
  }

  if (!SetHandleInformation(rd, handleFlagInherit, 0)) {
    return -1;
  }

  fd = _open_osfhandle(SUBPROCESS_PTR_CAST(subprocess_intptr_t, rd), 0);

  if (-1 != fd) {
    out_process->stdout_file = _fdopen(fd, "rb");

    if (SUBPROCESS_NULL == out_process->stdout_file) {
      return -1;
    }
  }

  startInfo.hStdOutput = wr;

  if (subprocess_option_combined_stdout_stderr == (options & subprocess_option_combined_stdout_stderr)) {
    out_process->stderr_file = out_process->stdout_file;
    startInfo.hStdError = startInfo.hStdOutput;
  } else {
    if (options & subprocess_option_enable_async) {
      if (subprocess_create_named_pipe_helper(&rd, &wr)) {
        return -1;
      }
    } else {
      if (!CreatePipe(&rd, &wr, SUBPROCESS_PTR_CAST(LPSECURITY_ATTRIBUTES, &saAttr), 0)) {
        return -1;
      }
    }

    if (!SetHandleInformation(rd, handleFlagInherit, 0)) {
      return -1;
    }

    fd = _open_osfhandle(SUBPROCESS_PTR_CAST(subprocess_intptr_t, rd), 0);

    if (-1 != fd) {
      out_process->stderr_file = _fdopen(fd, "rb");

      if (SUBPROCESS_NULL == out_process->stderr_file) {
        return -1;
      }
    }

    startInfo.hStdError = wr;
  }

  if (options & subprocess_option_enable_async) {
    out_process->hEventOutput
        = CreateEventA(SUBPROCESS_PTR_CAST(LPSECURITY_ATTRIBUTES, &saAttr), 1, 1, SUBPROCESS_NULL);
    out_process->hEventError = CreateEventA(SUBPROCESS_PTR_CAST(LPSECURITY_ATTRIBUTES, &saAttr), 1, 1, SUBPROCESS_NULL);
  } else {
    out_process->hEventOutput = SUBPROCESS_NULL;
    out_process->hEventError = SUBPROCESS_NULL;
  }

  // Combine commandLine together into a single string
  len = 0;
  for (i = 0; commandLine[i]; i++) {
    // for the trailing \0
    len++;

    // Quote the argument if it has a space in it
    if (strpbrk(commandLine[i], "\t\v ") != SUBPROCESS_NULL || commandLine[i][0] == SUBPROCESS_NULL)
      len += 2;

    for (j = 0; '\0' != commandLine[i][j]; j++) {
      switch (commandLine[i][j]) {
      default:
        break;
      case '\\':
        if (commandLine[i][j + 1] == '"') {
          len++;
        }

        break;
      case '"':
        len++;
        break;
      }
      len++;
    }
  }

  commandLineCombined = SUBPROCESS_CAST(char*, _alloca(len));

  if (!commandLineCombined) {
    return -1;
  }

  // Gonna re-use len to store the write index into commandLineCombined
  len = 0;

  for (i = 0; commandLine[i]; i++) {
    if (0 != i) {
      commandLineCombined[len++] = ' ';
    }

    need_quoting = strpbrk(commandLine[i], "\t\v ") != SUBPROCESS_NULL || commandLine[i][0] == SUBPROCESS_NULL;
    if (need_quoting) {
      commandLineCombined[len++] = '"';
    }

    for (j = 0; '\0' != commandLine[i][j]; j++) {
      switch (commandLine[i][j]) {
      default:
        break;
      case '\\':
        if (commandLine[i][j + 1] == '"') {
          commandLineCombined[len++] = '\\';
        }

        break;
      case '"':
        commandLineCombined[len++] = '\\';
        break;
      }

      commandLineCombined[len++] = commandLine[i][j];
    }
    if (need_quoting) {
      commandLineCombined[len++] = '"';
    }
  }

  commandLineCombined[len] = '\0';

  if (!CreateProcessA(SUBPROCESS_NULL,
          commandLineCombined, // command line
          SUBPROCESS_NULL, // process security attributes
          SUBPROCESS_NULL, // primary thread security attributes
          1, // handles are inherited
          flags, // creation flags
          used_environment, // used environment
          SUBPROCESS_NULL, // use parent's current directory
          SUBPROCESS_PTR_CAST(LPSTARTUPINFOA,
              &startInfo), // STARTUPINFO pointer
          SUBPROCESS_PTR_CAST(LPPROCESS_INFORMATION, &processInfo))) {
    return -1;
  }

  out_process->hProcess = processInfo.hProcess;

  out_process->hStdInput = startInfo.hStdInput;

  // We don't need the handle of the primary thread in the called process.
  CloseHandle(processInfo.hThread);

  if (SUBPROCESS_NULL != startInfo.hStdOutput) {
    CloseHandle(startInfo.hStdOutput);

    if (startInfo.hStdError != startInfo.hStdOutput) {
      CloseHandle(startInfo.hStdError);
    }
  }

  out_process->alive = 1;

  return 0;
#else
  int stdinfd[2];
  int stdoutfd[2];
  int stderrfd[2];
  pid_t child;
  extern char** environ;
  char* const empty_environment[1] = { SUBPROCESS_NULL };
  posix_spawn_file_actions_t actions;
  char* const* used_environment;

  if (subprocess_option_inherit_environment == (options & subprocess_option_inherit_environment)) {
    if (SUBPROCESS_NULL != environment) {
      return -1;
    }
  }

  if (0 != pipe(stdinfd)) {
    return -1;
  }

  if (0 != pipe(stdoutfd)) {
    return -1;
  }

  if (subprocess_option_combined_stdout_stderr != (options & subprocess_option_combined_stdout_stderr)) {
    if (0 != pipe(stderrfd)) {
      return -1;
    }
  }

  if (environment) {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif
    used_environment = SUBPROCESS_CONST_CAST(char* const*, environment);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
  } else if (subprocess_option_inherit_environment == (options & subprocess_option_inherit_environment)) {
    used_environment = environ;
  } else {
    used_environment = empty_environment;
  }

  if (0 != posix_spawn_file_actions_init(&actions)) {
    return -1;
  }

  // Close the stdin write end
  if (0 != posix_spawn_file_actions_addclose(&actions, stdinfd[1])) {
    posix_spawn_file_actions_destroy(&actions);
    return -1;
  }

  // Map the read end to stdin
  if (0 != posix_spawn_file_actions_adddup2(&actions, stdinfd[0], STDIN_FILENO)) {
    posix_spawn_file_actions_destroy(&actions);
    return -1;
  }

  // Close the stdout read end
  if (0 != posix_spawn_file_actions_addclose(&actions, stdoutfd[0])) {
    posix_spawn_file_actions_destroy(&actions);
    return -1;
  }

  // Map the write end to stdout
  if (0 != posix_spawn_file_actions_adddup2(&actions, stdoutfd[1], STDOUT_FILENO)) {
    posix_spawn_file_actions_destroy(&actions);
    return -1;
  }

  if (subprocess_option_combined_stdout_stderr == (options & subprocess_option_combined_stdout_stderr)) {
    if (0 != posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO)) {
      posix_spawn_file_actions_destroy(&actions);
      return -1;
    }
  } else {
    // Close the stderr read end
    if (0 != posix_spawn_file_actions_addclose(&actions, stderrfd[0])) {
      posix_spawn_file_actions_destroy(&actions);
      return -1;
    }
    // Map the write end to stdout
    if (0 != posix_spawn_file_actions_adddup2(&actions, stderrfd[1], STDERR_FILENO)) {
      posix_spawn_file_actions_destroy(&actions);
      return -1;
    }
  }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif
  if (subprocess_option_search_user_path == (options & subprocess_option_search_user_path)) {
    if (0
        != posix_spawnp(&child, commandLine[0], &actions, SUBPROCESS_NULL,
            SUBPROCESS_CONST_CAST(char* const*, commandLine), used_environment)) {
      posix_spawn_file_actions_destroy(&actions);
      return -1;
    }
  } else {
    if (0
        != posix_spawn(&child, commandLine[0], &actions, SUBPROCESS_NULL,
            SUBPROCESS_CONST_CAST(char* const*, commandLine), used_environment)) {
      posix_spawn_file_actions_destroy(&actions);
      return -1;
    }
  }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

  // Close the stdin read end
  close(stdinfd[0]);
  // Store the stdin write end
  out_process->stdin_file = fdopen(stdinfd[1], "wb");

  // Close the stdout write end
  close(stdoutfd[1]);
  // Store the stdout read end
  out_process->stdout_file = fdopen(stdoutfd[0], "rb");

  if (subprocess_option_combined_stdout_stderr == (options & subprocess_option_combined_stdout_stderr)) {
    out_process->stderr_file = out_process->stdout_file;
  } else {
    // Close the stderr write end
    close(stderrfd[1]);
    // Store the stderr read end
    out_process->stderr_file = fdopen(stderrfd[0], "rb");
  }

  // Store the child's pid
  out_process->child = child;

  out_process->alive = 1;

  posix_spawn_file_actions_destroy(&actions);
  return 0;
#endif
}

FILE* subprocess_stdin(const struct subprocess_s* const process)
{
  return process->stdin_file;
}

FILE* subprocess_stdout(const struct subprocess_s* const process)
{
  return process->stdout_file;
}

FILE* subprocess_stderr(const struct subprocess_s* const process)
{
  if (process->stdout_file != process->stderr_file) {
    return process->stderr_file;
  } else {
    return SUBPROCESS_NULL;
  }
}

int subprocess_join(struct subprocess_s* const process, int* const out_return_code)
{
#if defined(_WIN32)
  const unsigned long infinite = 0xFFFFFFFF;

  if (process->stdin_file) {
    fclose(process->stdin_file);
    process->stdin_file = SUBPROCESS_NULL;
  }

  if (process->hStdInput) {
    CloseHandle(process->hStdInput);
    process->hStdInput = SUBPROCESS_NULL;
  }

  WaitForSingleObject(process->hProcess, infinite);

  if (out_return_code) {
    if (!GetExitCodeProcess(process->hProcess, SUBPROCESS_PTR_CAST(unsigned long*, out_return_code))) {
      return -1;
    }
  }

  process->alive = 0;

  return 0;
#else
  int status;

  if (process->stdin_file) {
    fclose(process->stdin_file);
    process->stdin_file = SUBPROCESS_NULL;
  }

  if (process->child) {
    if (process->child != waitpid(process->child, &status, 0)) {
      return -1;
    }

    process->child = 0;

    if (WIFEXITED(status)) {
      process->return_status = WEXITSTATUS(status);
    } else {
      process->return_status = EXIT_FAILURE;
    }

    process->alive = 0;
  }

  if (out_return_code) {
    *out_return_code = process->return_status;
  }

  return 0;
#endif
}

int subprocess_destroy(struct subprocess_s* const process)
{
  if (process->stdin_file) {
    fclose(process->stdin_file);
    process->stdin_file = SUBPROCESS_NULL;
  }

  if (process->stdout_file) {
    fclose(process->stdout_file);

    if (process->stdout_file != process->stderr_file) {
      fclose(process->stderr_file);
    }

    process->stdout_file = SUBPROCESS_NULL;
    process->stderr_file = SUBPROCESS_NULL;
  }

#if defined(_WIN32)
  if (process->hProcess) {
    CloseHandle(process->hProcess);
    process->hProcess = SUBPROCESS_NULL;

    if (process->hStdInput) {
      CloseHandle(process->hStdInput);
    }

    if (process->hEventOutput) {
      CloseHandle(process->hEventOutput);
    }

    if (process->hEventError) {
      CloseHandle(process->hEventError);
    }
  }
#endif

  return 0;
}

int subprocess_terminate(struct subprocess_s* const process)
{
#if defined(_WIN32)
  unsigned int killed_process_exit_code;
  int success_terminate;
  int windows_call_result;

  killed_process_exit_code = 99;
  windows_call_result = TerminateProcess(process->hProcess, killed_process_exit_code);
  success_terminate = (windows_call_result == 0) ? 1 : 0;
  return success_terminate;
#else
  int result;
  result = kill(process->child, 9);
  return result;
#endif
}

unsigned subprocess_read_stdout(struct subprocess_s* const process, char* const buffer, unsigned size)
{
#if defined(_WIN32)
  void* handle;
  unsigned long bytes_read = 0;
  struct subprocess_overlapped_s overlapped = { 0, 0, { { 0, 0 } }, SUBPROCESS_NULL };
  overlapped.hEvent = process->hEventOutput;

  handle = SUBPROCESS_PTR_CAST(void*, _get_osfhandle(_fileno(process->stdout_file)));

  if (!ReadFile(handle, buffer, size, &bytes_read, SUBPROCESS_PTR_CAST(LPOVERLAPPED, &overlapped))) {
    const unsigned long errorIoPending = 997;
    unsigned long error = GetLastError();

    // Means we've got an async read!
    if (error == errorIoPending) {
      if (!GetOverlappedResult(handle, SUBPROCESS_PTR_CAST(LPOVERLAPPED, &overlapped), &bytes_read, 1)) {
        const unsigned long errorIoIncomplete = 996;
        const unsigned long errorHandleEOF = 38;
        error = GetLastError();

        if ((error != errorIoIncomplete) && (error != errorHandleEOF)) {
          return 0;
        }
      }
    }
  }

  return SUBPROCESS_CAST(unsigned, bytes_read);
#else
  const int fd = fileno(process->stdout_file);
  const ssize_t bytes_read = read(fd, buffer, size);

  if (bytes_read < 0) {
    return 0;
  }

  return SUBPROCESS_CAST(unsigned, bytes_read);
#endif
}

unsigned subprocess_read_stderr(struct subprocess_s* const process, char* const buffer, unsigned size)
{
#if defined(_WIN32)
  void* handle;
  unsigned long bytes_read = 0;
  struct subprocess_overlapped_s overlapped = { 0, 0, { { 0, 0 } }, SUBPROCESS_NULL };
  overlapped.hEvent = process->hEventError;

  handle = SUBPROCESS_PTR_CAST(void*, _get_osfhandle(_fileno(process->stderr_file)));

  if (!ReadFile(handle, buffer, size, &bytes_read, SUBPROCESS_PTR_CAST(LPOVERLAPPED, &overlapped))) {
    const unsigned long errorIoPending = 997;
    unsigned long error = GetLastError();

    // Means we've got an async read!
    if (error == errorIoPending) {
      if (!GetOverlappedResult(handle, SUBPROCESS_PTR_CAST(LPOVERLAPPED, &overlapped), &bytes_read, 1)) {
        const unsigned long errorIoIncomplete = 996;
        const unsigned long errorHandleEOF = 38;
        error = GetLastError();

        if ((error != errorIoIncomplete) && (error != errorHandleEOF)) {
          return 0;
        }
      }
    }
  }

  return SUBPROCESS_CAST(unsigned, bytes_read);
#else
  const int fd = fileno(process->stderr_file);
  const ssize_t bytes_read = read(fd, buffer, size);

  if (bytes_read < 0) {
    return 0;
  }

  return SUBPROCESS_CAST(unsigned, bytes_read);
#endif
}

int subprocess_alive(struct subprocess_s* const process)
{
  int is_alive = SUBPROCESS_CAST(int, process->alive);

  if (!is_alive) {
    return 0;
  }
#if defined(_WIN32)
  {
    const unsigned long zero = 0x0;
    const unsigned long wait_object_0 = 0x00000000L;

    is_alive = wait_object_0 != WaitForSingleObject(process->hProcess, zero);
  }
#else
  {
    int status;
    is_alive = 0 == waitpid(process->child, &status, WNOHANG);

    // If the process was successfully waited on we need to cleanup now.
    if (!is_alive) {
      if (WIFEXITED(status)) {
        process->return_status = WEXITSTATUS(status);
      } else {
        process->return_status = EXIT_FAILURE;
      }

      // Since we've already successfully waited on the process, we need to wipe
      // the child now.
      process->child = 0;

      if (subprocess_join(process, SUBPROCESS_NULL)) {
        return -1;
      }
    }
  }
#endif

  if (!is_alive) {
    process->alive = 0;
  }

  return is_alive;
}

#if defined(__clang__)
#if __has_warning("-Wunsafe-buffer-usage")
#pragma clang diagnostic pop
#endif
#endif

#if defined(__cplusplus)
} // extern "C"
#endif

#endif /* SHEREDOM_SUBPROCESS_H_INCLUDED */

// ======================================================================
//  *                   END Subprocess.h Library                       *
// ======================================================================

namespace ts {

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define clamp(a, b, c) max(min(a, c), b)

struct String;

[[noreturn]] void __panicStr(const char* str);
[[noreturn]] void __panicSizeT(const char* str, size_t value);
[[noreturn]] void __panicImpl(String str);
void* __memset(void* dest, int ch, size_t count);

void __format_vsnprintf(char* buffer, size_t bufferSize, const char* format, ...);
size_t __format_strlen(const char* str);
void __format_output_stdout(String string);
void __format_output_stderr(String string);

const auto DEFAULT_ARENA_SIZE = 16 * 1024 * 1024;
const auto MAX_PRINT_LINE_LENGTH = 4096;

struct ArenaChunk {
  ArenaChunk* nextChunk;
  size_t capacity;
  size_t used;
  // After here comes the data
  // dataPointer = chunkPointer + sizeof(ArenaChunk)
};

struct Arena {
  ArenaChunk* firstChunk;
  bool isStackArena;
  bool __initialized;

  [[nodiscard]] static Arena create(size_t chunkSize = DEFAULT_ARENA_SIZE);

  [[nodiscard]] static Arena createFromBuffer(char* buffer, size_t bufferSize);

  void enlarge(ArenaChunk** lastChunk, size_t chunkSize = DEFAULT_ARENA_SIZE);

  template <typename T> [[nodiscard]] T* allocate(size_t elementCount = 1)
  {
    if (!this->__initialized) {
      __panicStr("Arena was not properly initialized");
    }
    size_t size = elementCount * sizeof(T);
    ArenaChunk* lastChunk = this->firstChunk;
    while (lastChunk->nextChunk) {
      lastChunk = lastChunk->nextChunk;
    }

    if (lastChunk->capacity - lastChunk->used < size) {
      if (this->isStackArena) {
        __panicSizeT("Stack Arena is not large enough for allocation of size {}", size);
      } else {
        this->enlarge(&lastChunk, max(DEFAULT_ARENA_SIZE, size));
      }
    }

    uint8_t* data = (uint8_t*)lastChunk + sizeof(ArenaChunk) + lastChunk->used;
    lastChunk->used += size;
    return (T*)data;
  }

  void free();
  void clearAndReinit();
};

template <size_t Size>
// Note: The Arena is private and automatically converted, because the Arena cannot outlive the StackArena.
// Do not save the Arena from the StackArena out to a separate variable.
struct StackArena {
  char data[Size];

  [[nodiscard]] operator Arena&()
  {
    // Only valid as long as the address of this->data does not change
    if (!this->arenaInitialized) {
      this->_arena = Arena::createFromBuffer(this->data, Size);
      this->arenaInitialized = true;
    }
    return this->_arena;
  }

  template <typename T> [[nodiscard]] T* allocate(size_t elementCount = 1)
  {
    Arena& arena = *this;
    return arena.allocate<T>(elementCount);
  }

  private:
  Arena _arena = {};
  bool arenaInitialized = 0;
};

struct String {
  const char* data;
  size_t length;

  static const size_t npos = -1;

  [[nodiscard]] String concat(Arena& arena, String other);

  [[nodiscard]] size_t find(String delimiter, size_t startIndex = 0);

  [[nodiscard]] String substr(size_t startIndex, size_t count = npos) const;

  [[nodiscard]] char get(size_t index);

  [[nodiscard]] char operator[](size_t index);

  [[nodiscard]] const char* c_str(Arena& arena);

  [[nodiscard]] bool startsWith(String other) const;

  [[nodiscard]] bool endsWith(String other) const;

  [[nodiscard]] static String clone(Arena& arena, String string);

  [[nodiscard]] static String clone(Arena& arena, const char* str);

  [[nodiscard]] static String clone(Arena& arena, const char* str, size_t length);

  [[nodiscard]] static String view(const char* str);

  [[nodiscard]] static String view(const char* str, size_t length);

  [[nodiscard]] bool operator==(String other);

  String()
      : data(0)
      , length(0)
  {
  }

  String(const char* str)
  {
    *this = String::view(str);
  }

  String(const char* str, size_t length)
  {
    this->data = str;
    this->length = length;
  }
};

struct StringBuffer {
  char* data;
  size_t length;
  size_t capacity;

  static const size_t npos = -1;

  [[nodiscard]] char& get(size_t index);

  [[nodiscard]] char& operator[](size_t index);

  [[nodiscard]] const char* c_str(Arena& arena);

  [[nodiscard]] String str();

  [[nodiscard]] static StringBuffer clone(Arena& arena, String string);

  [[nodiscard]] static StringBuffer clone(Arena& arena, const char* str);

  [[nodiscard]] static StringBuffer clone(Arena& arena, const char* str, size_t length);

  [[nodiscard]] bool operator==(String other);

  StringBuffer& append(Arena& arena, char c);

  StringBuffer& append(Arena& arena, String str);

  [[nodiscard]] String findUntil(String criteria, size_t skip = 0);

  void enlarge(Arena& arena, size_t neededSize);

  StringBuffer()
      : data(0)
      , length(0)
      , capacity(0)
  {
  }

  StringBuffer(Arena& arena, const char* str)
  {
    *this = StringBuffer::clone(arena, str);
  }

  StringBuffer(Arena& arena, const char* str, size_t length)
  {
    *this = StringBuffer::clone(arena, str, length);
  }

  StringBuffer(Arena& arena, String str)
  {
    *this = StringBuffer::clone(arena, str);
  }
};

namespace literals {
  inline String operator""_s(const char* str, size_t length)
  {
    return String::view(str, length);
  }
}

template <typename... Args> [[noreturn]] void panic(const char* fmt, Args&&... args)
{
  Arena arena = Arena::create();
  String str = format(arena, fmt, args...);
  __panicImpl(str);
  arena.free();
}

template <typename T> struct Optional {

  Optional()
      : _hasValue(false)
  {
  }

  Optional(T value)
      : _value(value)
      , _hasValue(true)
  {
  }

  [[nodiscard]] T& value()
  {
    if (!this->_hasValue) {
      panic("Bad optional access");
    }
    return this->_value;
  }

  [[nodiscard]] const T& value_or(const T& defaultValue)
  {
    if (!this->_hasValue) {
      return defaultValue;
    }
    return this->_value;
  }

  [[nodiscard]] bool hasValue()
  {
    return this->_hasValue;
  }

  [[nodiscard]] operator bool()
  {
    return this->hasValue();
  }

  [[nodiscard]] T& operator*()
  {
    return this->value();
  }

  [[nodiscard]] T* operator->()
  {
    return &this->_value;
  }

  private:
  T _value;
  bool _hasValue;
};

template <typename TVal, typename TErr> struct Result {

  Result(TVal succ)
  {
    this->_value = succ;
    this->_hasValue = true;
  }

  Result(TErr err)
  {
    this->_error = err;
    this->_hasValue = false;
  }

  [[nodiscard]] TVal& value()
  {
    if (!this->_hasValue) {
      panic("Bad result access");
    }
    return this->_value;
  }

  [[nodiscard]] TVal value_or(TVal fallback)
  {
    if (!this->_hasValue) {
      return fallback;
    }
    return this->_value;
  }

  [[nodiscard]] TErr& error()
  {
    if (this->_hasValue) {
      panic("Bad result access");
    }
    return this->_error;
  }

  [[nodiscard]] operator bool()
  {
    return this->_hasValue;
  }

  [[nodiscard]] TVal& operator*()
  {
    return this->value();
  }

  [[nodiscard]] TVal* operator->()
  {
    return &this->_value;
  }

  private:
  union {
    TVal _value;
    TErr _error;
  };
  bool _hasValue;
};

template <typename T> struct ListElem {
  T data;
  ListElem<T>* nextElement;
};

template <typename T> struct List {
  size_t length = { 0 };

  template <typename V> void __add(Arena& arena, V v)
  {
    this->push(arena, v);
  }

  template <typename V, typename... U> void __add(Arena& arena, V v, U... u)
  {
    this->push(arena, v);
    __add(arena, u...);
  }

  List() = default;

  template <typename... U> constexpr List(Arena& arena, U... u)
  {
    __add(arena, u...);
  }

  void push(Arena& arena, T element)
  {
    if (!this->firstElement) {
      this->firstElement = arena.allocate<ListElem<T>>();
      this->firstElement->data = element;
    } else {
      ListElem<T>* lastElement = this->firstElement;
      while (lastElement->nextElement) {
        lastElement = lastElement->nextElement;
      }
      lastElement->nextElement = arena.allocate<ListElem<T>>();
      lastElement->nextElement->data = element;
    }
    this->length++;
  }

  void clear()
  {
    this->length = 0;
    this->firstElement = 0;
  }

  void pop()
  {
    if (!this->firstElement) {
      panic("Cannot pop from list: length = 0");
    }

    if (!this->firstElement->nextElement) {
      this->firstElement = 0;
      this->length = 0;
      return;
    }

    ListElem<T>* secondToLastElement = this->firstElement;
    while (secondToLastElement->nextElement->nextElement) {
      secondToLastElement = secondToLastElement->nextElement;
    }

    secondToLastElement->nextElement = 0;
    this->length--;
  }

  template <typename TFunc> void remove_if(TFunc&& pred)
  {
    while (firstElement && pred(firstElement->data)) {
      firstElement = firstElement->nextElement;
      length--;
    }

    ListElem<T>* curr = firstElement;
    while (curr && curr->nextElement) {
      if (pred(curr->nextElement->data)) {
        curr->nextElement = curr->nextElement->nextElement;
        length--;
      } else {
        curr = curr->nextElement;
      }
    }
  }

  [[nodiscard]] T& get(size_t index)
  {
    if (index < 0 || index >= this->length) {
      panic("List index out of bounds: {} >= {}", index, this->length);
    }
    ListElem<T>* elem = this->firstElement;
    for (size_t i = 0; i < index; i++) {
      elem = elem->nextElement;
    }
    return elem->data;
  }

  [[nodiscard]] T& operator[](size_t index)
  {
    return this->get(index);
  }

  [[nodiscard]] T& back()
  {
    if (this->length == 0) {
      panic("Cannot get back element of a zero element list");
    }
    return this->get(this->length - 1);
  }

  [[nodiscard]] bool contains(T val)
  {
    for (auto& elem : *this) {
      if (elem == val) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] Optional<size_t> findIndex(T val)
  {
    size_t index = 0;
    for (auto& elem : *this) {
      if (elem == val) {
        return index;
      }
      index++;
    }
    return {};
  }

  [[nodiscard]] List<T> reverse(Arena& arena)
  {
    List<T> result;

    for (auto it = begin(); it != end(); ++it) {
      auto* node = arena.allocate<ListElem<T>>();
      node->data = *it;
      node->nextElement = result.firstElement;
      result.firstElement = node;
      result.length++;
    }

    return result;
  }

  struct Iterator {
    ListElem<T>* current;
    Iterator(ListElem<T>* node)
        : current(node)
    {
    }

    T& operator*()
    {
      return current->data;
    }

    Iterator& operator++()
    {
      current = current->nextElement;
      return *this;
    }

    bool operator!=(const Iterator& other) const
    {
      return current != other.current;
    }
  };

  Iterator begin()
  {
    return Iterator(this->firstElement);
  }

  Iterator end()
  {
    return Iterator(nullptr);
  }

  private:
  ListElem<T>* firstElement = { 0 };
};

template <typename T, size_t Size> struct Array {
  [[nodiscard]] T* data()
  {
    return this->_data;
  }

  [[nodiscard]] T& get(size_t index)
  {
    if (index < 0 || index >= this->length()) {
      panic("Array index out of bounds: {} >= {}", index, this->length());
    }
    return this->_data[index];
  }

  [[nodiscard]] T& operator[](size_t index)
  {
    return this->get(index);
  }

  [[nodiscard]] size_t length()
  {
    return Size;
  }

  void zeroFill()
  {
    __memset(this->_data, 0, sizeof(this->_data));
  }

  private:
  T _data[Size];
};

template <typename T, typename U> struct Pair {
  T first;
  U second;
};

// ===========================================================
// ===                      FORMATTING                     ===
// ===========================================================

const int FORMAT_MAX_FORMATTER_LENGTH = 20;

template <typename T> struct remove_cv_ref {
  using type = T;
};

template <typename T> struct remove_cv_ref<const T> {
  using type = T;
};

template <typename T> struct remove_cv_ref<volatile T> {
  using type = T;
};

template <typename T> struct remove_cv_ref<const volatile T> {
  using type = T;
};

template <typename T> struct remove_cv_ref<T&> {
  using type = T;
};

template <typename T> struct remove_cv_ref<const T&> {
  using type = T;
};

template <typename T> struct remove_cv_ref<volatile T&> {
  using type = T;
};

template <typename T> struct remove_cv_ref<const volatile T&> {
  using type = T;
};

// Custom is_same implementation that ignores const, volatile, and references
template <typename T, typename U> struct is_same {
  static const bool value = false;
};

template <typename T> struct is_same<T, T> {
  static const bool value = true;
};

template <typename T, typename U>
inline constexpr bool is_same_v = is_same<typename remove_cv_ref<T>::type, typename remove_cv_ref<U>::type>::value;

struct false_type {
  static constexpr bool value = false;
};
struct true_type {
  static constexpr bool value = true;
};

template <typename T>
concept no_formatter = requires(T t) { T::value; };
template <typename T> struct formatter : false_type { };

static const char* makeFormatString(Arena& arena, String formatArg, char _default)
{
  if (formatArg.length == 0) {
    auto str = arena.allocate<char>(3);
    str[0] = '%';
    str[1] = _default;
    str[2] = '\0';
    return str;
  }
  auto length = formatArg.length;
  auto str = (char*)formatArg.c_str(arena);
  str[0] = '%';
  auto last = str[formatArg.length - 1];
  if (last != 'x' && last != 'X') {
    auto newStr = arena.allocate<char>(formatArg.length + 2);
    for (size_t i = 0; i < formatArg.length; i++) {
      newStr[i] = str[i];
    }
    newStr[formatArg.length] = _default;
    newStr[formatArg.length + 1] = '\0';
    return newStr;
  }
  return str;
}

template <> struct formatter<int> {
  static size_t format(const int& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    StackArena<64> arena;
    __format_vsnprintf(buffer, remainingBufferSize, makeFormatString(arena, formatArg, 'd'), value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<unsigned int> {
  static size_t format(const unsigned int& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%u", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<float> {
  static size_t format(const float& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%f", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<double> {
  static size_t format(const double& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%lf", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<long> {
  static size_t format(const long& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%ld", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<unsigned long> {
  static size_t format(const unsigned long& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%lu", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<short> {
  static size_t format(const short& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%d", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<unsigned short> {
  static size_t format(const unsigned short& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%u", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<bool> {
  static size_t format(const bool& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    if (value) {
      __format_vsnprintf(buffer, remainingBufferSize, "true");
      return __format_strlen(buffer);
    } else {
      __format_vsnprintf(buffer, remainingBufferSize, "false");
      return __format_strlen(buffer);
    }
  }
};

template <> struct formatter<String> {
  static size_t format(const String& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    for (size_t i = 0; i < value.length; i++) {
      if (i < remainingBufferSize) {
        buffer[i] = value.data[i];
      }
    }
    return value.length;
  }
};

template <> struct formatter<StringBuffer> {
  static size_t format(const StringBuffer& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    for (size_t i = 0; i < value.length; i++) {
      if (i < remainingBufferSize) {
        buffer[i] = value.data[i];
      }
    }
    return value.length;
  }
};

template <> struct formatter<const char*> {
  static size_t format(const char* const& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%s", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<char*> {
  static size_t format(char* value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%s", value);
    return __format_strlen(buffer);
  }
};

template <size_t LENGTH> struct formatter<char[LENGTH]> {
  static size_t format(const char value[LENGTH], String formatArg, char* buffer, size_t remainingBufferSize)
  {
    __format_vsnprintf(buffer, remainingBufferSize, "%s", value);
    return __format_strlen(buffer);
  }
};

template <> struct formatter<char> {
  static size_t format(const char& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    char buf[2] = { value, '\0' };
    __format_vsnprintf(buffer, remainingBufferSize, "%s", buf);
    return __format_strlen(buffer);
  }
};

template <typename T> size_t format_value(const T& value, String formatArg, char* buffer, size_t remainingBufferSize)
{
  using formatter_t = formatter<typename remove_cv_ref<T>::type>;
  if constexpr (no_formatter<formatter_t>) {
    static_assert(!no_formatter<formatter_t>, "No formatter is defined for this datatype");
    return 0;
  } else {
    return formatter_t::format(value, formatArg, buffer, remainingBufferSize);
  }
}

template <typename... Args>
void format_impl(String formatStr, int currentArg, int desiredArg, char* buf, size_t bufsize, size_t& currentLength)
{
}

template <typename T, typename... Args>
void format_impl(String formatStr, int currentArg, int desiredArg, char* buf, size_t bufsize, size_t& currentLength,
    T&& value, Args&&... args)
{
  if (currentArg == desiredArg) {
    size_t size = format_value(value, formatStr, buf + currentLength, bufsize - currentLength);
    currentLength += size;
  } else {
    format_impl(formatStr, currentArg + 1, desiredArg, buf, bufsize, currentLength, args...);
  }
}

inline void __format_concat(char* buf, size_t& buflen, size_t maxBufferLength, char* src, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    if (buflen < maxBufferLength) {
      buf[buflen++] = src[i];
    }
  }
}

template <size_t MaxSize = 8192, typename... Args> String format(Arena& arena, const char* fmt, Args&&... args)
{
  char buf[MaxSize];
  size_t buflen = 0;
  char formatStr[FORMAT_MAX_FORMATTER_LENGTH];
  size_t formatStrLength = 0;
  size_t argIndex = 0;

  bool isParsingArg = false;
  size_t fmtLen = __format_strlen(fmt);
  for (size_t i = 0; i < fmtLen; i++) {
    char c = fmt[i];
    char cnext = i != fmtLen - 1 ? fmt[i + 1] : '\0';
    if (!isParsingArg) {
      if (c == '{' && cnext != '{') {
        isParsingArg = true;
        formatStrLength = 0;
      } else if (c == '{' && cnext == '{') {
        __format_concat(buf, buflen, MaxSize, &c, 1);
        i++;
      } else if (c == '}' && cnext == '}') {
        __format_concat(buf, buflen, MaxSize, &c, 1);
        i++;
      } else {
        __format_concat(buf, buflen, MaxSize, &c, 1);
      }
    } else {
      if (c == '}') {
        isParsingArg = false;
        format_impl(String::view(formatStr, formatStrLength), 0, argIndex, buf, MaxSize, buflen, args...);
        argIndex++;
      } else {
        if (formatStrLength < FORMAT_MAX_FORMATTER_LENGTH) {
          formatStr[formatStrLength++] = fmt[i];
        }
      }
    }
  }
  return String::clone(arena, buf, buflen);
}

template <size_t MaxSize = 8192, typename... Args> void print(const char* fmt, Args&&... args)
{
  using namespace ts::literals;
  StackArena<MaxSize> arena;
  String result = format(arena, fmt, args...);
  __format_output_stdout(result);
  __format_output_stdout("\n"_s);
}

template <size_t MaxSize = 8192, typename... Args> void print_nnl(const char* fmt, Args&&... args)
{
  StackArena<MaxSize> arena;
  String result = format(arena, fmt, args...);
  __format_output_stdout(result);
}

template <size_t MaxSize = 8192, typename... Args> void print_stderr(const char* fmt, Args&&... args)
{
  using namespace ts::literals;
  StackArena<MaxSize> arena;
  String result = format(arena, fmt, args...);
  __format_output_stderr(result);
  __format_output_stderr("\n"_s);
}

template <typename T> class _Vec2 {
  public:
  T x;
  T y;

  _Vec2()
  {
    this->x = 0;
    this->y = 0;
  }

  _Vec2(T x, T y)
  {
    this->x = x;
    this->y = y;
  }

  double length()
  {
    return sqrt(pow(this->x, 2) + pow(this->y, 2));
  }

  _Vec2 operator-(const _Vec2& other)
  {
    return _Vec2(this->x - other.x, this->y - other.y);
  }

  _Vec2 operator+(const _Vec2& other)
  {
    return _Vec2(this->x + other.x, this->y + other.y);
  }

  _Vec2 operator*(const _Vec2& other)
  {
    return _Vec2(this->x * other.x, this->y * other.y);
  }

  _Vec2 operator/(const _Vec2& other)
  {
    return _Vec2(this->x / other.x, this->y / other.y);
  }

  _Vec2 operator*(T value)
  {
    return _Vec2(this->x * value, this->y * value);
  }

  _Vec2 operator/(T value)
  {
    return _Vec2(this->x / value, this->y / value);
  }

  _Vec2 operator-=(const _Vec2& other)
  {
    this->x -= other.x;
    this->y -= other.y;
    return *this;
  }

  _Vec2 operator+=(const _Vec2& other)
  {
    this->x += other.x;
    this->y += other.y;
    return *this;
  }

  _Vec2 operator*=(const _Vec2& other)
  {
    this->x *= other.x;
    this->y *= other.y;
    return *this;
  }

  _Vec2 operator/=(const _Vec2& other)
  {
    this->x /= other.x;
    this->y /= other.y;
    return *this;
  }

  _Vec2 operator*=(T v)
  {
    this->x *= v;
    this->y *= v;
    return *this;
  }

  _Vec2 operator/=(T v)
  {
    this->x /= v;
    this->y /= v;
    return *this;
  }

  _Vec2 operator-()
  {
    return _Vec2(-this->x, -this->y);
  }

  _Vec2 normalize()
  {
    return *this / this->length();
  }

  bool operator==(const _Vec2& v)
  {
    return this->x == v.x && this->y == v.y;
  }

  bool operator!=(const _Vec2& v)
  {
    return !((*this) == v);
  }
};

using Vec2 = _Vec2<double>;
using Vec2i = _Vec2<int>;

class Mat4 {
  public:
  Array<float, 16> data;

  Mat4()
  {
    this->data.zeroFill();
  }

  static Mat4 Identity()
  {
    Mat4 mat = Mat4();
    mat.data[0] = 1;
    mat.data[5] = 1;
    mat.data[10] = 1;
    mat.data[15] = 1;
    return mat;
  }

  void multiply(Mat4 other)
  {
    float result[16] = { 0 };

    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        result[i * 4 + j] = data[i * 4 + 0] * other.data[0 * 4 + j] + data[i * 4 + 1] * other.data[1 * 4 + j]
            + data[i * 4 + 2] * other.data[2 * 4 + j] + data[i * 4 + 3] * other.data[3 * 4 + j];
      }
    }

    // Copy the result into current matrix
    for (int i = 0; i < 16; i++) {
      data[i] = result[i];
    }
  }

  // Apply translation (tx, ty, tz)
  void applyTranslation(float tx, float ty, float tz)
  {
    Mat4 translationMatrix = Mat4::Identity();

    translationMatrix.data[3] = tx; // Set translation values
    translationMatrix.data[7] = ty;
    translationMatrix.data[11] = tz;

    multiply(translationMatrix); // Multiply current matrix with translation
  }

  // Apply scaling (sx, sy, sz)
  void applyScaling(float sx, float sy, float sz)
  {
    Mat4 scalingMatrix = Mat4::Identity();

    scalingMatrix.data[0] = sx; // Set scaling values
    scalingMatrix.data[5] = sy;
    scalingMatrix.data[10] = sz;

    multiply(scalingMatrix); // Multiply current matrix with scaling
  }

  void print()
  {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        ts::print_nnl("{} ", data[i * 4 + j]);
      }
      ts::print("");
    }
  }
};

struct Color;
enum struct ParseError { NonDigitsRemaining };
[[nodiscard]] Result<int64_t, ParseError> strToInt(String string);
[[nodiscard]] Result<double, ParseError> strToDouble(String string);
[[nodiscard]] uint8_t hexToDigit(char letter);
[[nodiscard]] Color hexToColor(ts::String hex);
[[nodiscard]] ts::String colorToHex(Arena& arena, Color color);

struct Color {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t a = 0;

  Color()
  {
    this->r = 0;
    this->g = 0;
    this->b = 0;
    this->a = 0;
  }

  Color(float r, float g, float b, float a = 255)
  {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }

#ifdef TINYSTD_USE_CLAY
  Color(Clay_Color color)
  {
    this->r = color.r;
    this->g = color.g;
    this->b = color.b;
    this->a = color.a;
  }
#endif

  Color(String color)
  {
    *this = hexToColor(color);
  }

  Color(const char* color)
  {
    *this = hexToColor(String::view(color));
  }

#ifdef TINYSTD_USE_CLAY
  operator Clay_Color()
  {
    Clay_Color color = {
      .r = r,
      .g = g,
      .b = b,
      .a = a,
    };
    return color;
  }
#endif // USE_CLAY
};

template <> struct formatter<Color> {
  static size_t format(const Color& value, String formatArg, char* buffer, size_t remainingBufferSize)
  {
    StackArena<8192> arena;
    auto col = colorToHex(arena, value);
    __format_vsnprintf(buffer, remainingBufferSize, "%s", col.c_str(arena), col.length);
    return __format_strlen(buffer);
  }
};

struct Subprocess {
  struct subprocess_s subprocess;

  struct JoinResult {
    size_t exitCode;
    StringBuffer stdout;
    StringBuffer stderr;
  };

  static Result<Subprocess, String> create(List<String> args)
  {
    StackArena<1024> arena;
    const char** cmds = arena.allocate<const char*>(args.length + 1);
    for (size_t i = 0; i < args.length; i++) {
      cmds[i] = args[i].c_str(arena);
    }
    cmds[args.length] = NULL;

    Subprocess process;
    int result = subprocess_create(cmds, subprocess_option_inherit_environment, &process.subprocess);
    if (0 != result) {
      return String::view(strerror(errno));
    }
    return process;
  }

  Result<JoinResult, String> join(Arena& arena)
  {
    int return_code;
    int joinResult = subprocess_join(&this->subprocess, &return_code);
    if (0 != joinResult) {
      auto err = String::view(strerror(errno));
      subprocess_terminate(&subprocess);
      subprocess_destroy(&subprocess);
      return err;
    }

    FILE* p_stdout = stdout();
    StringBuffer stdout;
    while (true) {
      char buffer[8192];
      auto result = fgets(buffer, sizeof(buffer), p_stdout);
      if (result == NULL) {
        break;
      }
      stdout.append(arena, buffer);
    }

    FILE* p_stderr = stderr();
    StringBuffer stderr;
    while (true) {
      char buffer[8192];
      auto result = fgets(buffer, sizeof(buffer), p_stderr);
      if (result == NULL) {
        break;
      }
      stderr.append(arena, buffer);
    }

    int destroyResult = subprocess_destroy(&subprocess);
    if (0 != destroyResult) {
      return String::view(strerror(errno));
    }

    return { { .exitCode = 0, .stdout = stdout, .stderr = stderr } };
  }

  FILE* stdin()
  {
    return subprocess_stdin(&subprocess);
  }

  FILE* stdout()
  {
    return subprocess_stdout(&subprocess);
  }

  FILE* stderr()
  {
    return subprocess_stderr(&subprocess);
  }

  void readStdoutToBuffer(Arena& arena, StringBuffer& stringBuffer);
  void writeToStdin(String str);
};

namespace fs {

  bool exists(String path);

  Optional<String> read(Arena& arena, String path);

} // namespace fs

} // namespace ts

// ===========================================================
// ===                    IMPLEMENTATION                   ===
// ===========================================================

#ifdef TINYSTD_IMPLEMENTATION

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ts {

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
Arena Arena::create(size_t chunkSize)
{
  size_t allocSize = sizeof(ArenaChunk) + chunkSize;
  Arena newArena;
  newArena.firstChunk = (ArenaChunk*)calloc(allocSize, 1);
  newArena.isStackArena = false;
  newArena.__initialized = true;
  if (newArena.firstChunk == 0) {
    __panicSizeT("Arena chunk allocation of size {} failed", allocSize);
  }
  newArena.firstChunk->capacity = chunkSize;
  return newArena;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
Arena Arena::createFromBuffer(char* buffer, size_t bufferSize)
{
  if (bufferSize <= sizeof(ArenaChunk)) {
    __panicStr("Cannot create arena from buffer: Buffer too small");
  }
  ArenaChunk* chunk = (ArenaChunk*)buffer;
  chunk->nextChunk = 0;
  chunk->capacity = bufferSize - sizeof(ArenaChunk);
  chunk->used = 0;
  memset((uint8_t*)chunk + sizeof(ArenaChunk), 0, chunk->capacity);

  Arena arena;
  arena.firstChunk = chunk;
  arena.isStackArena = true;
  arena.__initialized = true;
  return arena;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void Arena::enlarge(ArenaChunk** lastChunk, size_t chunkSize)
{
  print("Warning: Arena was enlarged. Consider more short-lived arenas to prevent excessive memory usage.");
  if (!this->__initialized) {
    __panicStr("Arena was not properly initialized");
  }
  if (this->isStackArena) {
    __panicStr("Cannot enlarge a stack-based arena");
  }
  size_t allocSize = sizeof(ArenaChunk) + chunkSize;
  (*lastChunk)->nextChunk = (ArenaChunk*)calloc(allocSize, 1);
  if ((*lastChunk)->nextChunk == 0) {
    __panicSizeT("Arena chunk allocation of size {} failed", allocSize);
  }
  (*lastChunk) = (*lastChunk)->nextChunk;
  (*lastChunk)->capacity = chunkSize;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void Arena::free()
{
  ArenaChunk* current = this->firstChunk;
  while (current) {
    ArenaChunk* next = current->nextChunk;
    ::free(current);
    current = next;
  }
  this->firstChunk = 0;
  this->isStackArena = false;
  this->__initialized = false;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void Arena::clearAndReinit()
{
  if (!this->__initialized) {
    *this = Arena::create();
  } else {
    // Clear every page except the first, and reset the first page
    // This means when reiniting an arena, most of the time no allocation is necessary
    ArenaChunk* current = this->firstChunk->nextChunk;
    while (current) {
      ArenaChunk* next = current->nextChunk;
      ::free(current);
      current = next;
    }
    this->firstChunk->nextChunk = 0;
    this->firstChunk->used = 0;
    memset((uint8_t*)this->firstChunk + sizeof(ArenaChunk), 0, this->firstChunk->capacity);
  }
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::concat(Arena& arena, String other)
{
  char* buf = arena.allocate<char>(this->length + other.length + 1);
  memcpy(buf, this->data, this->length);
  memcpy(buf + this->length, other.data, other.length);
  return String(buf, this->length + other.length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
size_t String::find(String delimiter, size_t startIndex)
{
  if (delimiter.length == 0 || startIndex >= length) {
    return npos;
  }
  for (size_t i = startIndex; i <= length - delimiter.length; ++i) {
    if (memcmp(data + i, delimiter.data, delimiter.length) == 0) {
      return i;
    }
  }
  return npos;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::substr(size_t startIndex, size_t count) const
{
  if (startIndex >= length) {
    return { nullptr, 0 };
  }

  if (count == npos || startIndex + count > length) {
    count = length - startIndex;
  }

  return { data + startIndex, count };
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
char String::get(size_t index)
{
  if (index >= this->length) {
    __panicStr("String index access out of range");
  }
  return this->data[index];
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
char String::operator[](size_t index)
{
  return this->get(index);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
const char* String::c_str(Arena& arena)
{
  String s = String::clone(arena, *this);
  return s.data;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
bool String::startsWith(String other) const
{
  if (this->length < other.length) {
    return false;
  }
  for (size_t i = 0; i < other.length; i++) {
    if (this->data[i] != other.data[i]) {
      return false;
    }
  }
  return true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
bool String::endsWith(String other) const
{
  if (this->length < other.length) {
    return false;
  }
  for (size_t i = 0; i < other.length; i++) {
    if (this->data[this->length - other.length + i] != other.data[i]) {
      return false;
    }
  }
  return true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::clone(Arena& arena, String string)
{
  char* buf = arena.allocate<char>(string.length + 1);
  memcpy(buf, string.data, string.length);
  return String(buf, string.length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::clone(Arena& arena, const char* str)
{
  return String::clone(arena, str, strlen(str));
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::clone(Arena& arena, const char* str, size_t length)
{
  char* buf = arena.allocate<char>(length + 1);
  memcpy(buf, str, length);
  return String(buf, length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::view(const char* str)
{
  return String(str, strlen(str));
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
String String::view(const char* str, size_t length)
{
  return String(str, length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
bool String::operator==(String other)
{
  if (this->length != other.length) {
    return false;
  }
  return 0 == memcmp(this->data, other.data, this->length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
char& StringBuffer::get(size_t index)
{
  if (index >= this->length) {
    __panicStr("StringBuffer index access out of range");
  }
  return this->data[index];
}

char& StringBuffer::operator[](size_t index)
{
  return this->get(index);
}

const char* StringBuffer::c_str(Arena& arena)
{
  return String::view(this->data, this->length).c_str(arena);
}

String StringBuffer::str()
{
  return String::view(this->data, this->length);
}

StringBuffer StringBuffer::clone(Arena& arena, String string)
{
  StringBuffer buf;
  buf.data = arena.allocate<char>(string.length * 2);
  memcpy(buf.data, string.data, string.length);
  buf.length = string.length;
  buf.capacity = string.length * 2;
  return buf;
}

StringBuffer StringBuffer::clone(Arena& arena, const char* str)
{
  return StringBuffer::clone(arena, String::view(str));
}

StringBuffer StringBuffer::clone(Arena& arena, const char* str, size_t length)
{
  return StringBuffer::clone(arena, String::view(str, length));
}

bool StringBuffer::operator==(String other)
{
  return String::view(this->data, this->length) == other;
}

StringBuffer& StringBuffer::append(Arena& arena, char c)
{
  if (this->capacity < this->length + 1) {
    this->enlarge(arena, this->length + 1);
  }
  this->data[this->length] = c;
  this->length++;
  return *this;
}

StringBuffer& StringBuffer::append(Arena& arena, String str)
{
  if (this->capacity < this->length + str.length) {
    this->enlarge(arena, this->length + str.length);
  }
  memcpy(this->data + this->length, str.data, str.length);
  this->length += str.length;
  return *this;
}

String StringBuffer::findUntil(String criteria, size_t skip)
{
  char* ptr = this->data + skip;
  size_t lengthRemaining = this->length - skip;

  while (true) {
    if (lengthRemaining == 0) {
      return String::view(this->data + skip, this->length - skip);
    }
    String remainingStr = String::view(ptr, lengthRemaining);
    if (remainingStr.startsWith(criteria)) {
      return String::view(this->data + skip, remainingStr.data - (this->data + skip));
    }
    ptr++;
    lengthRemaining--;
  }
}

void StringBuffer::enlarge(Arena& arena, size_t neededSize)
{
  if (this->capacity == 0) {
    this->capacity = max(8, neededSize * 2);
    this->data = arena.allocate<char>(this->capacity);
    return;
  }
  this->capacity = max(this->capacity * 2, neededSize * 2);
  char* newBuffer = arena.allocate<char>(this->capacity);
  memcpy(newBuffer, this->data, this->length);
  this->data = newBuffer;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __panicImpl(String str)
{
  print_stderr("[FATAL] Thread panicked: {}", str);
  fflush(stderr);
  abort();
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __format_vsnprintf(char* buffer, size_t bufferSize, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, bufferSize, format, args);
  va_end(args);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
size_t __format_strlen(const char* str)
{
  return strlen(str);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void* __memset(void* dest, int ch, size_t count)
{
  return memset(dest, ch, count);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __format_output_stdout(String string)
{
  fwrite(string.data, 1, string.length, stdout);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __format_output_stderr(String string)
{
  fwrite(string.data, 1, string.length, stderr);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __panicStr(const char* str)
{
  panic("{}", str);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void __panicSizeT(const char* str, size_t value)
{
  panic(str, value);
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
Result<int64_t, ParseError> strToInt(String string)
{
  char* endptr;
  StackArena<64> tmpArena;
  if (string.length >= 64) {
    panic("ts::strToInt(): String to be converted is too long to parse to integer");
  }
  int64_t value = strtol(string.c_str(tmpArena), &endptr, 10);
  if (*endptr != '\0') {
    return ParseError::NonDigitsRemaining;
  }
  return value;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
Result<double, ParseError> strToDouble(String string)
{
  char* endptr;
  StackArena<64> tmpArena;
  if (string.length >= 64) {
    panic("ts::strToDouble(): String to be converted is too long to parse to double: '{}'", string);
  }
  double value = strtod(string.c_str(tmpArena), &endptr);
  if (*endptr != '\0') {
    return ParseError::NonDigitsRemaining;
  }
  return value;
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void Subprocess::readStdoutToBuffer(Arena& arena, StringBuffer& stringBuffer)
{
  int fd = fileno(this->stdout());
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  while (true) {
    char buf[1024];
    ssize_t n = read(fd, buf, sizeof(buf));
    if (n > 0) {
      stringBuffer.append(arena, { buf, n });
    } else {
      break;
    }
  }
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
void Subprocess::writeToStdin(String str)
{
  fwrite(str.data, sizeof(char), str.length, this->stdin());
  fflush(this->stdin());
}

namespace fs {

  // NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
  bool exists(String path)
  {
    char pathStr[path.length + 1];
    memcpy(pathStr, path.data, path.length);
    pathStr[path.length] = '\0';
    struct stat sb;
    return (stat(pathStr, &sb) == 0);
  }

  // NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
  Optional<String> read(Arena& arena, String path)
  {
    FILE* f = fopen(path.c_str(arena), "rb");
    if (!f) {
      return {};
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);

    char* buf = arena.allocate<char>(sz);
    if (!buf) {
      fclose(f);
      return {};
    }

    if (fread(buf, 1, sz, f) != (size_t)sz) {
      fclose(f);
      return {};
    }
    fclose(f);

    return String::view(buf, sz);
  }

} // namespace fs

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
uint8_t hexToDigit(char letter)
{
  if (letter >= '0' && letter <= '9') {
    return letter - '0';
  } else if (letter >= 'a' && letter <= 'f') {
    return letter - 'a' + 10;
  } else if (letter >= 'A' && letter <= 'F') {
    return letter - 'A' + 10;
  } else {
    return 0;
  }
}

// NOLINTNEXTLINE(misc-definitions-in-headers) -> Implementation Macro is used
Color hexToColor(String hex)
{
  if (hex.length == 4) {
    uint8_t r = hexToDigit(hex[1]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[2]) + 16 * hexToDigit(hex[2]);
    uint8_t b = hexToDigit(hex[3]) + 16 * hexToDigit(hex[3]);
    uint8_t a = 255;
    return Color(r, g, b, a);
  } else if (hex.length == 5) {
    uint8_t r = hexToDigit(hex[1]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[2]) + 16 * hexToDigit(hex[2]);
    uint8_t b = hexToDigit(hex[3]) + 16 * hexToDigit(hex[3]);
    uint8_t a = hexToDigit(hex[4]) + 16 * hexToDigit(hex[4]);
    return Color(r, g, b, a);
  } else if (hex.length == 7) {
    uint8_t r = hexToDigit(hex[2]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[4]) + 16 * hexToDigit(hex[3]);
    uint8_t b = hexToDigit(hex[6]) + 16 * hexToDigit(hex[5]);
    uint8_t a = 255;
    return Color(r, g, b, a);
  } else if (hex.length == 9) {
    uint8_t r = hexToDigit(hex[2]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[4]) + 16 * hexToDigit(hex[3]);
    uint8_t b = hexToDigit(hex[6]) + 16 * hexToDigit(hex[5]);
    uint8_t a = hexToDigit(hex[8]) + 16 * hexToDigit(hex[7]);
    return Color(r, g, b, a);
  } else {
    return Color(0, 0, 0, 0);
  }
}

ts::String colorToHex(Arena& arena, Color color)
{
  return format(arena, "#{:02X}{:02X}{:02X}{:02X}", (int)color.r, (int)color.g, (int)color.b, (int)color.a);
}

} // namespace ts

#endif // TINYSTD_IMPLEMENTATION

#endif // TINYSTD_H
