#ifndef APP_H
#define APP_H

#include "clay.h"
#include "std.h"
#include "vec.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_surface.h>
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

struct Page {
  List<Shape> s;
  SDL_Surface* canvas = 0;

  Page() { }

  ~Page()
  {
    if (this->canvas) {
      SDL_DestroySurface(this->canvas);
    }
  }
};

struct Document {
  int pageWidthPixels = 0;
  int pageScroll = 0;
  List<Page> pages;
};

struct ClayVideoDemo_Arena {
  intptr_t offset;
  intptr_t memory;
};

struct Clay_SDL3RendererData {
  SDL_Renderer* renderer;
  TTF_TextEngine* textEngine;
  List<Tuple<TTF_Font*, int>> fonts;
};

struct UICache;

struct Appstate;
typedef void (*DrawUI_t)(Appstate* appstate);
typedef SDL_AppResult (*EventHandler_t)(Appstate* appstate, SDL_Event* event);
typedef void (*InitClay_t)(Appstate* appstate);

struct Appstate {
  Tool tool;
  SDL_Window* window = 0;
  SDL_Surface* mainDocumentRenderSurface;
  Vec2 mainDocumentRenderSurfaceSize;
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
  UICache* uiCache;
};

#endif // APP_H