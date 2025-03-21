
#include "std.h"
#include "format.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Arena Arena::create(size_t chunkSize)
{
  size_t allocSize = sizeof(ArenaChunk) + chunkSize;
  Arena newArena;
  newArena.firstChunk = (ArenaChunk*)calloc(allocSize, 1);
  newArena.isStackArena = false;
  newArena.__initialized = true;
  if (newArena.firstChunk == 0) {
    panicSizeT("Arena chunk allocation of size {} failed", allocSize);
  }
  newArena.firstChunk->capacity = chunkSize;
  print("Creating arena");
  return newArena;
}

Arena Arena::createFromBuffer(char* buffer, size_t bufferSize)
{
  if (bufferSize <= sizeof(ArenaChunk)) {
    panicStr("Cannot create arena from buffer: Buffer too small");
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

void Arena::enlarge(ArenaChunk** lastChunk, size_t chunkSize)
{
  print("Warning: Arena was enlarged. Consider more short-lived arenas to prevent excessive memory usage.");
  if (!this->__initialized) {
    panicStr("Arena was not properly initialized");
  }
  if (this->isStackArena) {
    panicStr("Cannot enlarge a stack-based arena");
  }
  size_t allocSize = sizeof(ArenaChunk) + chunkSize;
  (*lastChunk)->nextChunk = (ArenaChunk*)calloc(allocSize, 1);
  if ((*lastChunk)->nextChunk == 0) {
    panicSizeT("Arena chunk allocation of size {} failed", allocSize);
  }
  (*lastChunk) = (*lastChunk)->nextChunk;
  (*lastChunk)->capacity = chunkSize;
}

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

String String::concat(Arena& arena, String other)
{
  char* buf = arena.allocate<char>(this->length + other.length + 1);
  memcpy(buf, this->data, this->length);
  memcpy(buf + this->length, other.data, other.length);
  return String(buf, this->length + other.length);
}

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

char String::get(size_t index)
{
  if (index >= this->length) {
    panicStr("String index access out of range");
  }
  return this->data[index];
}

char String::operator[](size_t index)
{
  return this->get(index);
}

const char* String::c_str(Arena& arena)
{
  String s = String::clone(arena, *this);
  return s.data;
}

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

String String::clone(Arena& arena, String string)
{
  char* buf = arena.allocate<char>(string.length + 1);
  memcpy(buf, string.data, string.length);
  return String(buf, string.length);
}

String String::clone(Arena& arena, const char* str)
{
  return String::clone(arena, str, strlen(str));
}

String String::clone(Arena& arena, const char* str, size_t length)
{
  char* buf = arena.allocate<char>(length + 1);
  memcpy(buf, str, length);
  return String(buf, length);
}

String String::view(const char* str)
{
  return String(str, strlen(str));
}

String String::view(const char* str, size_t length)
{
  return String(str, length);
}

bool String::operator==(String other)
{
  if (this->length != other.length) {
    return false;
  }
  return 0 == memcmp(this->data, other.data, this->length);
}

void __panicImpl(String str)
{
  print_stderr("[FATAL] Thread panicked: {}", str);
  fflush(stderr);
  abort();
}

void __format_vsnprintf(char* buffer, size_t bufferSize, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, bufferSize, format, args);
  va_end(args);
}

size_t __format_strlen(const char* str)
{
  return strlen(str);
}

void* __memset(void* dest, int ch, size_t count)
{
  return memset(dest, ch, count);
}

void __format_output_stdout(String string)
{
  fwrite(string.data, 1, string.length, stdout);
}

void __format_output_stderr(String string)
{
  fwrite(string.data, 1, string.length, stderr);
}