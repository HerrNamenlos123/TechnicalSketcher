#ifndef APP_H
#define APP_H

#include "clay.h"
#include "std.h"
#include "vec.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cwchar>
#include <dlfcn.h>
#include <filesystem>
#include <functional>
#include <stdint.h>

enum class Tool {
  Select,
  Pen,
};

struct LineShape { };
using Shape = Variant<LineShape>;

struct Document {
  List<Shape> s;
  Tool tool;
  SDL_Texture* canvas = 0;
};

struct ClayVideoDemo_Arena {
  intptr_t offset;
  intptr_t memory;
};

struct Clay_SDL3RendererData {
  SDL_Renderer* renderer;
  TTF_TextEngine* textEngine;
  List<TTF_Font*> fonts;
};

struct Appstate;
typedef Clay_RenderCommandArray (*DrawUI_t)(Appstate* appstate);
typedef SDL_AppResult (*EventHandler_t)(Appstate* appstate, SDL_Event* event);
typedef void (*InitClay_t)(Appstate* appstate);

struct Appstate {
  SDL_Window* window = 0;
  Clay_SDL3RendererData rendererData;
  List<Document> documents = {};
  uint64_t lastHotreloadUpdate = 0;
  Map<String, fs::file_time_type> fileModificationDates;
  bool compileError = false;
  void* appLibraryHandle = 0;
  DrawUI_t DrawUI = 0;
  EventHandler_t EventHandler = 0;
  InitClay_t InitClay = 0;
  Clay_Context* clayContext;
};

#endif // APP_H