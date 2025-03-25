#define SDL_MAIN_USE_CALLBACKS 1
#include "../GL/glad.h"
#include "../shared/app.h"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "hotreload.cpp"

const Vec2 DEFAULT_WINDOW_SIZE = Vec2(1920, 1080);

static SDL_AppResult AppLoop(App* app)
{
  app->frameArena.clearAndReinit();
  if (auto result = UpdateHotreload(app); result != SDL_APP_CONTINUE) {
    return result;
  }

  glClearColor(0, 0, 0, 255);
  glClear(GL_COLOR_BUFFER_BIT);

  if (!app->compileError && app->RenderApp) {
    app->RenderApp(app);
  } else {
    glClearColor(255, 0, 0, 255);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  SDL_GL_SwapWindow(app->window);

  return SDL_APP_CONTINUE;
}

static SDL_AppResult InitApp(App* app)
{
  app->lastHotreloadUpdate = SDL_GetTicks();
  SDL_SetAppMetadata("Example Pen Drawing Lines", "1.0", "com.example.pen-drawing-lines");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  app->window = SDL_CreateWindow("examples/pen/drawing-lines", (int)DEFAULT_WINDOW_SIZE.x, (int)DEFAULT_WINDOW_SIZE.y,
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!app->window) {
    SDL_Log("Couldn't create window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  app->glContext = SDL_GL_CreateContext(app->window);
  if (!app->glContext) {
    SDL_Log("Couldn't create OpenGL context");
    return SDL_APP_FAILURE;
  }

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    SDL_Log("Couldn't load GLAD");
  }

  if (!TTF_Init()) {
    return SDL_APP_FAILURE;
  }

  // app->rendererData.textEngine = TTF_CreateRendererTextEngine(app->rendererData.renderer);
  // if (!app->rendererData.textEngine) {
  //   SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create text engine from renderer: %s", SDL_GetError());
  //   return SDL_APP_FAILURE;
  // }

  StackArena<1024> arena;
  List<int> sizes;
  sizes.push(arena, 12);
  sizes.push(arena, 14);
  sizes.push(arena, 16);
  sizes.push(arena, 18);
  sizes.push(arena, 20);
  sizes.push(arena, 24);
  app->rendererData.fonts = app->persistentApplicationArena.allocate<FontData>(sizes.length);
  app->rendererData.numberOfFonts = 0;
  for (auto size : sizes) {
    TTF_Font* font = TTF_OpenFont("resource/Roboto-Regular.ttf", size);
    if (!font) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }
    app->rendererData.fonts[app->rendererData.numberOfFonts++] = { font, size };
  }

  compileApp(app);
  if (app->compileError) {
    return SDL_APP_FAILURE;
  }

  if (!loadAppLib(app)) {
    return SDL_APP_FAILURE;
  }

  if (app->LoadApp) {
    app->LoadApp(app, true);
  } else {
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

static void DestroyApp(App* app)
{
  if (app->UnloadApp) {
    app->UnloadApp(app);
  }

  for (size_t i = 0; i < app->rendererData.numberOfFonts; i++) {
    TTF_CloseFont(app->rendererData.fonts[i].font);
  }

  // if (app->rendererData.textEngine) {
  //   TTF_DestroyRendererTextEngine(app->rendererData.textEngine);
  // }

  SDL_GL_DestroyContext(app->glContext);

  if (app->window) {
    SDL_DestroyWindow(app->window);
  }

  app->clayArena.free();
  app->frameArena.free();

  // Shallow copy the arena, because otherwise the method
  // would free its own this pointer
  Arena arena = app->persistentApplicationArena;
  arena.free();

  TTF_Quit();
}

SDL_AppResult SDL_AppInit(void** _app, int argc, char* argv[])
{
  Arena mainArena = Arena::create();
  *_app = mainArena.allocate<App>();
  ((App*)(*_app))->persistentApplicationArena = mainArena;
  ((App*)(*_app))->frameArena.clearAndReinit();
  return InitApp((App*)(*_app));
}

SDL_AppResult SDL_AppEvent(void* _app, SDL_Event* event)
{
  App* app = (App*)_app;
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  if (app->EventHandler) {
    return app->EventHandler(app, event);
  } else {
    return SDL_APP_CONTINUE;
  }
}

SDL_AppResult SDL_AppIterate(void* _app)
{
  App* app = (App*)_app;
  return AppLoop(app);
}

void SDL_AppQuit(void* _app, SDL_AppResult result)
{
  App* app = (App*)_app;
  if (result != SDL_APP_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Application failed to run");
  }

  if (app) {
    DestroyApp(app);
  }
}
