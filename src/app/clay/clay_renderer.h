
#ifndef CLAY_RENDERER_H
#define CLAY_RENDERER_H

#include "../../shared/clay.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  TTF_Font* font;
  int size;
} FontData;

typedef struct {
  TTF_TextEngine* textEngine;
  FontData* fonts;
  size_t numberOfFonts;
} RendererData;

void SDL_Clay_RenderClayCommands(RendererData* rendererData, Clay_RenderCommandArray* rcommands);

#ifdef __cplusplus
}
#endif

#endif // CLAY_RENDERER_H