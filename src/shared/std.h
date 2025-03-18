
#ifndef BASE_H
#define BASE_H

#include "format.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define tsk_min(a, b) (((a) < (b)) ? (a) : (b))
#define tsk_max(a, b) (((a) > (b)) ? (a) : (b))

const auto DEFAULT_ARENA_SIZE = 16 * 1024 * 1024;

struct Arena {
  Arena* nextChunk;
  Arena* prevChunk;
  Arena* lastChunk;
  void* data;
  size_t capacity;
  size_t used;

  static Arena* create(size_t chunkSize = DEFAULT_ARENA_SIZE)
  {
    Arena* newArena = (Arena*)calloc(sizeof(Arena), 1);
    if (newArena == 0) {
      fprintf(stderr, "Memory allocation unexpectedly failed");
      abort();
    }
    newArena->data = (Arena*)calloc(chunkSize, 1);
    if (newArena->data == 0) {
      fprintf(stderr, "Memory allocation unexpectedly failed");
      abort();
    }
    newArena->capacity = chunkSize;
    newArena->lastChunk = newArena;
    return newArena;
  }

  void
  enlarge(size_t chunkSize = DEFAULT_ARENA_SIZE)
  {
    Arena* lastChunk = this->lastChunk;
    Arena* newArena = Arena::create(chunkSize);
    lastChunk->nextChunk = newArena;
    newArena->prevChunk = lastChunk;

    Arena* current = newArena;
    while (current) {
      current->lastChunk = newArena;
      current = current->prevChunk;
    }
  }

  template <typename T>
  T* allocate(size_t elementCount = 1)
  {
    size_t size = elementCount * sizeof(T);
    Arena* lastChunk = this->lastChunk;
    if (lastChunk->capacity - lastChunk->used < size) {
      printf("Warning: Arena was enlarged. Consider more short-lived arenas to prevent excessive memory usage.");
      this->enlarge(tsk_max(DEFAULT_ARENA_SIZE, size));
    }
    lastChunk = lastChunk->lastChunk;

    if (lastChunk->capacity - lastChunk->used < size) {
      fprintf(stderr, "[Impossible] Arena enlarged but still not large enough");
      abort();
    }

    uint8_t* data = (uint8_t*)lastChunk->data + lastChunk->used;
    lastChunk->used += size;
    return (T*)data;
  }
};

static void FreeArena(Arena* arena)
{
  Arena* first = arena;
  while (first->prevChunk) {
    first = first->prevChunk;
  }

  Arena* current = first;
  while (current) {
    Arena* next = current->nextChunk;
    free(current->data);
    free(current);
    current = next;
  }
}

struct String {
  const char* data;
  size_t length;

  String(const char* str)
  {
    this->data = str,
    this->length = strlen(str);
  }

  String(const char* str, size_t length)
  {
    this->data = str,
    this->length = length;
  }

  String concat(Arena* arena, String other)
  {
    char* buf = arena->allocate<char>(this->length + other.length + 1);
    memcpy(buf, this->data, this->length);
    memcpy(buf + this->length, other.data, other.length);
    return String(buf, this->length + other.length);
  }

  static String allocate(Arena* arena, const char* str, size_t length)
  {
    char* buf = arena->allocate<char>(length + 1);
    memcpy(buf, str, length);
    return String(buf, length);
  }
};

static char* c_str(Arena* arena, String string)
{
  char* str = arena->allocate<char>(string.length + 1);
  memcpy(str, string.data, string.length);
  str[string.length] = 0;
  return str;
}

template <typename T>
struct ListElem {
  T data;
  ListElem<T>* nextElement;
};

template <typename T>
struct List {
  ListElem<T>* firstElement;
  size_t length;

  static List<T>* create(Arena* arena)
  {
    List<T>* list = arena->allocate<List<T>>();
    list->firstElement = 0;
    list->length = 0;
    return list;
  }

  void push(Arena* arena, T element)
  {
    if (!this->firstElement) {
      this->firstElement = arena->allocate<ListElem<T>>();
      this->firstElement->data = element;
    } else {
      ListElem<T>* lastElement = this->firstElement;
      while (lastElement->nextElement) {
        lastElement = lastElement->nextElement;
      }
      lastElement->nextElement = arena->allocate<ListElem<T>>();
      lastElement->nextElement->data = element;
    }
  }

  void pop()
  {
    // if (!this->firstElement) {
    //   this->firstElement = arena->allocate<ListElem<T>>();
    //   this->firstElement->data = element;
    // } else {
    //   ListElem<T>* lastElement = this->firstElement;
    //   while (lastElement->nextElement) {
    //     lastElement = lastElement->nextElement;
    //   }
    //   lastElement->nextElement = arena->allocate<ListElem<T>>();
    //   lastElement->nextElement->data = element;
    // }
  }
};

static void foo(Arena* arena)
{
  auto array = List<int>::create(arena);
  // array.push();
}

#endif // BASE_H