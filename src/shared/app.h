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

#include "gl.hpp"
#include "shared.h"

using ts::Color;
using ts::Mat4;
using ts::Optional;
using ts::Pair;
using ts::print;
using ts::Vec2;
using ts::Vec2f;
using ts::Vec2i;
using ts::Vec3;
using ts::Vec3f;
using namespace ts::literals;

enum class Tool {
  Select,
  Pen,
};

struct resvg_options;

struct SamplePoint {
  Vec2 pos_mm_scaled;
  float pressure;
};

struct LineShape {
  List<SamplePoint> points;
  Color color;
  bool prerendered;
};

struct Document;
struct App;

struct Page {
  Document* document;
  size_t pageNumId;
  List<LineShape> shapes;
  gl::Texture tempRenderTexture;
  gl::Framebuffer persistentFBO;
  Vec2 visibleSizePx;
  Vec2 visibleOffsetPx;
  gl::Framebuffer previewFBO;

  Vec2i getRenderSizePx(App* app);
  Vec2i getTopLeftPx(App* app);
  bool overlapsWithViewport(App* app);
};

struct Document {
  float zoomMmPerPx = {};
  int pageScroll = {};
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
  float perfectFreehandAccuracyScaling;
  float penPressureScaling;

  // Device input
  List<SDL_TouchFingerEvent> touchFingers;
  Optional<Vec2> prevAveragePos;
  Optional<float> prevPinchDistance;

  struct {
    bool ctrl;
    bool shift;
    bool mousewheel;
  } inputs;

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

  // UI
  List<Pair<String, Color>> colors;
  List<Pair<String, int>> fonts;

  // OpenGL
  bool recreateGlTexture;
  SDL_GLContext glContext;
  GLuint mainShader;
  GLuint lineshapeShader;
  GLuint mainViewportVAO;
  GLuint mainViewportVBO;
  GLuint mainViewportIBO;
  GLuint mainViewportRBO;
  GLuint mainViewportTEX;

  // SVG
  resvg_options* svgOpts;

  // Profiling
  struct Profiler {
    struct Result {
      const char* scopeName;
      double msTaken;
    };
    ts::Array<Result, 100> results;
    size_t numOfResults;
    double frametimeMs;
  };
  Profiler lastFrameProfilingResults;
  Profiler currentProfilingResults;
};

#endif // APP_H