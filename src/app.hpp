#ifndef APP_H
#define APP_H

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clay_renderer.cpp"

#include "std.hpp"
#include "vec.hpp"
#include <SDL3/SDL.h>
// #include <SDL3/SDL_image.h>
#include <SDL3/SDL_main.h>
// #include <SDL3/SDL_ttf.h>
#include <stdint.h>
#include <stdlib.h>

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

struct Appstate {
  SDL_Window* window = 0;
  Clay_SDL3RendererData rendererData;
  ClayVideoDemo_Arena clayFrameArena;
  List<Document> documents = {};
};

#endif // APP_H