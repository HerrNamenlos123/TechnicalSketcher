#ifndef APP_H
#define APP_H

#include "clay.h"
#include "std.h"
#include "stdlib.h"
#include "vector.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdint.h>

#include "shared.h"

enum class Tool {
  Select,
  Pen,
};

// All units are always in mm
struct InterpolationPoint {
  Vec2 pos;
  float thickness;
};

struct LineShape {
  // List<InterpolationPoint> points;
  Color color;
};
// using Shape = Variant<LineShape>;

struct Page {
  // List<Shape> shapes;
  SDL_Texture* canvas = 0;
};

struct Document {
  float pageWidthPercentOfWindow = 50;
  int pageScroll = 0;
  Vec2 position;
  // List<Page> pages;
  Color paperColor = {};
};

struct ClayVideoDemo_Arena {
  intptr_t offset;
  intptr_t memory;
};

struct Clay_SDL3RendererData {
  SDL_Renderer* renderer;
  TTF_TextEngine* textEngine;
  // List<Tuple<TTF_Font*, int>> fonts;
};

struct UICache;

struct App;
typedef void (*DrawUI_t)(App* appstate);
typedef SDL_AppResult (*EventHandler_t)(App* appstate, SDL_Event* event);
typedef void (*InitClay_t)(App* appstate);

struct App {
  // Actual application data
  // List<Document> documents = {};
  size_t selectedDocument = 0;
  Tool tool;
  float currentPenPressure = 0;
  LineShape currentLine;
  int currentlyDrawingOnPage = 0;

  // Constants
  const float pageGapPercentOfHeight = 3;

  // Device input
  // List<SDL_TouchFingerEvent> touchFingers;
  // Optional<Vec2> prevAveragePos;
  // Optional<float> prevPinchDistance;

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
  // Map<String, time_t> fileModificationDates;
  SDL_Window* window;
  SDL_Texture* mainDocumentRenderTexture;
  Clay_BoundingBox mainViewportBB;

  // Memory stuff
  Arena* mainArena;
};

#endif // APP_H