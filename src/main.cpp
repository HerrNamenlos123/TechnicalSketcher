#define SDL_MAIN_USE_CALLBACKS 1
#include "app.hpp"
#include <stdio.h>

#include "renderer.cpp"

const Vec2 DEFAULT_WINDOW_SIZE = Vec2(640, 480);

static inline Clay_Dimensions SDL_MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, void* _appstate)
{
  Appstate* appstate = (Appstate*)_appstate;
  auto& fonts = appstate->rendererData.fonts;
  TTF_Font* font = fonts[config->fontId];
  int width, height;

  if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s", SDL_GetError());
  }

  return (Clay_Dimensions) { (float)width, (float)height };
}

void HandleClayErrors(Clay_ErrorData errorData) { printf("%s", errorData.errorText.chars); }

SDL_AppResult SDL_AppInit(void** _appstate, int argc, char* argv[])
{
  *_appstate = new Appstate();
  if (!*_appstate) {
    return SDL_APP_FAILURE;
  }
  auto appstate = (Appstate*)(*_appstate);

  SDL_SetAppMetadata("Example Pen Drawing Lines", "1.0", "com.example.pen-drawing-lines");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("examples/pen/drawing-lines",
                                   DEFAULT_WINDOW_SIZE.x,
                                   DEFAULT_WINDOW_SIZE.y,
                                   0,
                                   &appstate->window,
                                   &appstate->rendererData.renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetWindowResizable(appstate->window, true);

  if (!TTF_Init()) {
    return SDL_APP_FAILURE;
  }

  appstate->rendererData.textEngine = TTF_CreateRendererTextEngine(appstate->rendererData.renderer);
  if (!appstate->rendererData.textEngine) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create text engine from renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  TTF_Font* font = TTF_OpenFont("resource/Roboto-Regular.ttf", 24);
  if (!font) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  appstate->rendererData.fonts.push_back(font);

  uint64_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = (Clay_Arena) { .capacity = totalMemorySize, .memory = (char*)SDL_malloc(totalMemorySize) };

  int width, height;
  SDL_GetWindowSize(appstate->window, &width, &height);
  Clay_Initialize(
      clayMemory, (Clay_Dimensions) { (float)width, (float)height }, (Clay_ErrorHandler) { HandleClayErrors });
  Clay_SetMeasureTextFunction(SDL_MeasureText, appstate);

  int w, h;
  SDL_GetRenderOutputSize(appstate->rendererData.renderer, &w, &h);
  SDL_SetRenderDrawColor(appstate->rendererData.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(appstate->rendererData.renderer);
  SDL_SetRenderTarget(appstate->rendererData.renderer, NULL);
  SDL_SetRenderDrawBlendMode(appstate->rendererData.renderer, SDL_BLENDMODE_BLEND);

  //   appstate->documents.emplace_back();
  //   appstate->documents[0].canvas
  //       = SDL_CreateTexture(appstate->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
  //   if (!render_target) {
  //     SDL_Log("Couldn't create render target: %s", SDL_GetError());
  //     return SDL_APP_FAILURE;
  //   }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* _appstate, SDL_Event* event)
{
  Appstate* appstate = (Appstate*)_appstate;
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }

  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
    break;

  case SDL_EVENT_WINDOW_RESIZED:
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)event->window.data1, (float)event->window.data2 });
    break;

  case SDL_EVENT_MOUSE_MOTION:
    Clay_SetPointerState((Clay_Vector2) { event->motion.x, event->motion.y }, event->motion.state & SDL_BUTTON_LMASK);
    break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    Clay_SetPointerState((Clay_Vector2) { event->button.x, event->button.y }, event->button.button == SDL_BUTTON_LEFT);
    break;

  case SDL_EVENT_MOUSE_WHEEL:
    Clay_UpdateScrollContainers(true, (Clay_Vector2) { event->wheel.x, event->wheel.y }, 0.01f);
    break;

  default:
    break;
  }

  /* There are several events that track the specific stages of pen activity,
     but we're only going to look for motion and pressure, for simplicity. */
  if (event->type == SDL_EVENT_PEN_MOTION) {
    /* you can check for when the pen is touching, but if pressure > 0.0f, it's definitely touching! */
    // if (pressure > 0.0f) {
    //   if (previous_touch_x >= 0.0f) { /* only draw if we're moving while touching */
    /* draw with the alpha set to the pressure, so you effectively get a fainter line for lighter presses. */
    // SDL_SetRenderTarget(appstate->renderer, render_target);
    SDL_SetRenderDrawColorFloat(appstate->rendererData.renderer, 0, 0, 0, 500);
    SDL_RenderLine(appstate->rendererData.renderer,
                   event->pmotion.x - 10,
                   event->pmotion.y - 10,
                   event->pmotion.x,
                   event->pmotion.y);
    printf("Output\n");
    //   }
    //   previous_touch_x = event->pmotion.x;
    //   previous_touch_y = event->pmotion.y;
    // }
    // else {
    //   previous_touch_x = previous_touch_y = -1.0f;
    // }
  }
  else if (event->type == SDL_EVENT_PEN_AXIS) {
    // if (event->paxis.axis == SDL_PEN_AXIS_PRESSURE) {
    //   pressure = event->paxis.value; /* remember new pressure for later draws. */
    // }
    // else if (event->paxis.axis == SDL_PEN_AXIS_XTILT) {
    //   tilt_x = event->paxis.value;
    // }
    // else if (event->paxis.axis == SDL_PEN_AXIS_YTILT) {
    //   tilt_y = event->paxis.value;
    // }
  }

  return SDL_APP_CONTINUE;
}

Clay_RenderCommandArray DrawUI(Appstate* appstate)
{
  appstate->clayFrameArena.offset = 0;
  Clay_BeginLayout();
  Clay_Sizing layoutExpand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };

  // Build UI here

  Clay_RenderCommandArray renderCommands = Clay_EndLayout();
  // for (int32_t i = 0; i < renderCommands.length; i++) {
  //   Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y += data->yOffset;
  // }
  return renderCommands;
}

SDL_AppResult SDL_AppIterate(void* _appstate)
{
  Appstate* appstate = (Appstate*)_appstate;

  Clay_RenderCommandArray render_commands = DrawUI(appstate);

  SDL_SetRenderDrawColor(appstate->rendererData.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(appstate->rendererData.renderer);

  SDL_Clay_RenderClayCommands(&appstate->rendererData, &render_commands);

  SDL_RenderPresent(appstate->rendererData.renderer);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* _appstate, SDL_AppResult result)
{
  Appstate* appstate = (Appstate*)_appstate;

  if (result != SDL_APP_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Application failed to run");
  }

  if (appstate) {

    for (size_t i = 0; i < appstate->rendererData.fonts.size(); i++) {
      TTF_CloseFont(appstate->rendererData.fonts[i]);
    }

    if (appstate->rendererData.textEngine) {
      TTF_DestroyRendererTextEngine(appstate->rendererData.textEngine);
    }

    if (appstate->rendererData.renderer) {
      SDL_DestroyRenderer(appstate->rendererData.renderer);
    }

    if (appstate->window) {
      SDL_DestroyWindow(appstate->window);
    }
    delete appstate;
  }
  TTF_Quit();
}
