#ifndef APP_H
#define APP_H

#include "clay.h"
#include "std.h"
#include "vec.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
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
  SDL_Texture* canvas = 0;

  Page() { }

  ~Page()
  {
    if (this->canvas) {
      SDL_DestroyTexture(this->canvas);
    }
  }

  Page(const Page&) = delete;
  Page(Page&& other)
  {
    this->s = std::move(other.s);
    this->canvas = other.canvas;
    other.canvas = 0;
  }

  void operator=(const Page&) = delete;
  void operator=(Page&& other)
  {
    this->s = std::move(other.s);
    this->canvas = other.canvas;
    other.canvas = 0;
  };
};

struct Document {
  float pageWidthPercentOfWindow = 50;
  int pageScroll = 0;
  Vec2 position;
  List<Page> pages;
  Color paperColor = {};
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

  // Actual application data
  List<Document> documents = {};
  size_t selectedDocument = 0;
  Tool tool;

  // Device input
  List<SDL_TouchFingerEvent> touchFingers;
  Optional<Vec2> prevAveragePos;
  Optional<float> prevPinchDistance;

  // Hotreloading and UI stuff
  Clay_SDL3RendererData rendererData;
  uint64_t lastHotreloadUpdate = 0;
  bool compileError = false;
  void* appLibraryHandle = 0;
  DrawUI_t DrawUI = 0;
  EventHandler_t EventHandler = 0;
  InitClay_t InitClay = 0;
  UICache* uiCache;
  Clay_Context* clayContext;
  Map<String, fs::file_time_type> fileModificationDates;
  SDL_Window* window = 0;
  SDL_Texture* mainDocumentRenderTexture;
  Clay_BoundingBox mainViewportBB;
};

#endif // APP_H