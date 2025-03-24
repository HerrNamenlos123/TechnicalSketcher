#ifndef APP_H
#define APP_H

#include "../app/clay/clay_renderer.h"
#include "clay.h"
#include "std.h"
#include "stdlib.h"
#include "vector.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_opengl.h>
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
  List<InterpolationPoint> points;
  Color color;
};
// using Shape = Variant<LineShape>;

struct Page {
  List<LineShape> shapes;
};

struct Document {
  float pageWidthPercentOfWindow = 0;
  int pageScroll = 0;
  Vec2 position = {};
  List<Page> pages = {};
  Color paperColor = {};
  LineShape currentLine;
  Arena arena;
};

struct ClayVideoDemo_Arena {
  intptr_t offset;
  intptr_t memory;
};

struct UICache;

struct App;
typedef void (*DrawUI_t)(App* app);
typedef SDL_AppResult (*EventHandler_t)(App* app, SDL_Event* event);
typedef void (*InitApp_t)(App* app);
typedef void (*ResyncApp_t)(App* app);
typedef void (*DestroyApp_t)(App* app);

struct App {
  // Actual application data
  List<Document> documents = {};
  size_t selectedDocument = 0;
  Tool tool;
  float currentPenPressure = 0;
  int currentlyDrawingOnPage = 0;

  // Constants
  const float pageGapPercentOfHeight = 3;

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
  InitApp_t InitApp = 0;
  ResyncApp_t ResyncApp = 0;
  DestroyApp_t DestroyApp = 0;
  UICache* uiCache;
  Clay_Context* clayContext;
  List<Pair<String, time_t>> fileModificationDates;
  SDL_Window* window;
  SDL_Texture* mainViewportRenderTexture;
  Clay_BoundingBox mainViewportBB;

  SDL_Texture* pageRenderTarget = 0;
  SDL_Texture* pageSoftwareTexture = 0;

  // Memory stuff
  Arena persistentApplicationArena;
  Arena frameArena;
  Arena clayArena;

  // OpenGL
  SDL_GLContext glContext;
  GLuint lineshapeShaderprogram;
};

#endif // APP_H