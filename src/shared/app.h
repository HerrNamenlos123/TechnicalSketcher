#ifndef APP_H
#define APP_H

#define TINYSTD_USE_CLAY
#include "../GL/glad.h"
#include "../app/clay/clay_renderer.h"
#include "TinyStd.hpp"
#include "clay.h"
#include "math.h"
#include "stdlib.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdint.h>

#include "shared.h"

using ts::Color;
using ts::Mat4;
using ts::Optional;
using ts::Pair;
using ts::Vec2;
using namespace ts::literals;

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
typedef SDL_AppResult (*EventHandler_t)(App* app, SDL_Event* event);
typedef void (*LoadApp_t)(App* app, bool firstLoad);
typedef void (*RenderApp_t)(App* app);
typedef void (*UnloadApp_t)(App* app);

struct App {
  // Actual application data
  List<Document> documents;
  size_t selectedDocument;
  Tool tool;
  float currentPenPressure;
  int currentlyDrawingOnPage;

  // Constants
  float pageGapPercentOfHeight;

  // Device input
  List<SDL_TouchFingerEvent> touchFingers;
  Optional<Vec2> prevAveragePos;
  Optional<float> prevPinchDistance;

  // Hotreloading and UI stuff
  RendererData rendererData;
  uint64_t lastHotreloadUpdate;
  bool compileError;
  void* appLibraryHandle;
  EventHandler_t EventHandler;
  LoadApp_t LoadApp;
  RenderApp_t RenderApp;
  UnloadApp_t UnloadApp;
  UICache* uiCache;
  Clay_Context* clayContext;
  List<Pair<String, time_t>> fileModificationDates;
  SDL_Window* window;
  Vec2 windowSize;
  Clay_BoundingBox mainViewportBB;

  // Memory stuff
  Arena persistentApplicationArena;
  Arena frameArena;
  Arena clayArena;

  // OpenGL
  bool recreateGlTexture;
  SDL_GLContext glContext;
  GLuint mainShader;
  GLuint lineshapeShader;
  GLuint mainViewportVAO;
  GLuint mainViewportFBO;
  GLuint mainViewportVBO;
  GLuint mainViewportIBO;
  GLuint mainViewportRBO;
  GLuint mainViewportTEX;
};

#endif // APP_H