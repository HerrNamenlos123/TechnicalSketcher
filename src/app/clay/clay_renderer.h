
#ifndef CLAY_RENDERER_H
#define CLAY_RENDERER_H

#include "../../GL/glad.h"
#include "../../shared/clay.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "../font/fontstash.h"
#include "../font/glfontstash.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int* fonts;
  size_t numberOfFonts;
  FONScontext* fontContext;
  int windowWidth;
  int windowHeight;

  // OpenGL
  SDL_GLContext glContext;
  GLuint mainShader;
  GLuint uiVAO;
  GLuint uiVBO;
  GLuint uiIBO;
} RendererData;

void SDL_Clay_RenderClayCommands(RendererData* rendererData, Clay_RenderCommandArray* rcommands);

#ifdef __cplusplus
}
#endif

#endif // CLAY_RENDERER_H