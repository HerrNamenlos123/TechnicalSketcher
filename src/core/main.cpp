#define SDL_MAIN_USE_CALLBACKS 1
#include "../shared/app.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <dlfcn.h>
#include <stdio.h>

const Vec2 DEFAULT_WINDOW_SIZE = Vec2(1920, 1080);
const uint64_t HOTRELOAD_UPDATE_RATE = 100;

bool compileApp(App* appstate)
{
  printf("Application code changed, recompiling...\n");
  auto result = system("cmake --build build --target app");
  if (result != 0) {
    printf("Failed to build app!\n");
    appstate->compileError = true;
    return false;
  }

  printf("Done, reloaded app!\n");
  appstate->compileError = false;
  return true;
}

void closeAppLib(App* appstate)
{
  if (appstate->appLibraryHandle) {
    appstate->DrawUI = 0;
    appstate->EventHandler = 0;
    appstate->InitApp = 0;
    appstate->DestroyApp = 0;
    dlclose(appstate->appLibraryHandle);
    appstate->appLibraryHandle = 0;
  }
}

bool loadAppLib(App* app)
{
  closeAppLib(app);

  app->appLibraryHandle = dlopen("build/libapp.so", RTLD_LAZY);
  if (!app->appLibraryHandle) {
    printf("Error loading library: %s", dlerror());
    return false;
  }

  app->DrawUI = (DrawUI_t)dlsym(app->appLibraryHandle, "DrawUI");
  if (app->DrawUI == 0) {
    printf("Failed to load func: %s\n", dlerror());
    app->compileError = true;
    return false;
  }

  app->EventHandler = (EventHandler_t)dlsym(app->appLibraryHandle, "EventHandler");
  if (app->EventHandler == 0) {
    printf("Failed to load func: %s\n", dlerror());
    app->compileError = true;
    return false;
  }

  app->InitApp = (InitApp_t)dlsym(app->appLibraryHandle, "InitApp");
  if (app->InitApp == 0) {
    printf("Failed to load func: %s\n", dlerror());
    app->compileError = true;
    return false;
  }

  app->DestroyApp = (InitApp_t)dlsym(app->appLibraryHandle, "DestroyApp");
  if (app->DestroyApp == 0) {
    printf("Failed to load func: %s\n", dlerror());
    app->compileError = true;
    return false;
  }

  app->DestroyApp(app);
  app->InitApp(app);

  // Clear any previous errors
  dlerror();
  return true;
}

static SDL_AppResult UpdateHotreload(App* app)
{
  auto now = SDL_GetTicks();
  auto elapsed = now - app->lastHotreloadUpdate;
  if (elapsed > HOTRELOAD_UPDATE_RATE) {
    app->lastHotreloadUpdate = now;

    bool reloadApp = false;
    if (auto cwd = GetCurrentWorkingDirectory(app->frameArena)) {
      auto result = ListDirectory(app->frameArena, format(app->frameArena, "{}/src/app", cwd.value_or(""s)));
      for (auto file : result.value_or({})) {
        if (IsRegularFile(app->frameArena, file).value_or(false)) {
          auto moddate = GetFileModificationDate(file).value_or(0);

          bool found = false;
          for (auto& [filename, timestamp] : app->fileModificationDates) {
            if (filename == file) {
              found = true;
              if (timestamp != moddate) {
                timestamp = moddate;
                reloadApp = true;
              }
              break;
            }
          }
          if (!found) {
            app->fileModificationDates.push(app->persistentApplicationArena, { String::clone(app->persistentApplicationArena, file), moddate });
          }
        }
      }
    }

    bool reloadCore = false;
    if (auto cwd = GetCurrentWorkingDirectory(app->frameArena)) {
      auto result = ListDirectory(app->frameArena, format(app->frameArena, "{}/src/core", cwd.value_or(""s)));
      for (auto file : result.value_or({})) {
        if (IsRegularFile(app->frameArena, file).value_or(false)) {
          auto moddate = GetFileModificationDate(file).value_or(0);

          bool found = false;
          for (auto& [filename, timestamp] : app->fileModificationDates) {
            if (filename == file) {
              found = true;
              if (timestamp != moddate) {
                timestamp = moddate;
                reloadCore = true;
              }
              break;
            }
          }
          if (!found) {
            app->fileModificationDates.push(app->persistentApplicationArena, { String::clone(app->persistentApplicationArena, file), moddate });
          }
        }
      }
      result = ListDirectory(app->frameArena, format(app->frameArena, "{}/src/shared", cwd.value_or(""s)));
      for (auto file : result.value_or({})) {
        if (IsRegularFile(app->frameArena, file).value_or(false)) {
          auto moddate = GetFileModificationDate(file).value_or(0);

          bool found = false;
          for (auto& [filename, timestamp] : app->fileModificationDates) {
            if (filename == file) {
              found = true;
              if (timestamp != moddate) {
                timestamp = moddate;
                reloadCore = true;
              }
              break;
            }
          }
          if (!found) {
            app->fileModificationDates.push(app->persistentApplicationArena, { String::clone(app->persistentApplicationArena, file), moddate });
          }
        }
      }
      result = ListDirectory(app->frameArena, format(app->frameArena, "{}/src/shared/platform", cwd.value_or(""s)));
      for (auto file : result.value_or({})) {
        if (IsRegularFile(app->frameArena, file).value_or(false)) {
          auto moddate = GetFileModificationDate(file).value_or(0);

          bool found = false;
          for (auto& [filename, timestamp] : app->fileModificationDates) {
            if (filename == file) {
              found = true;
              if (timestamp != moddate) {
                timestamp = moddate;
                reloadCore = true;
              }
              break;
            }
          }
          if (!found) {
            app->fileModificationDates.push(app->persistentApplicationArena, { String::clone(app->persistentApplicationArena, file), moddate });
          }
        }
      }
    }

    if (reloadCore) {
      printf("Core code changed, application must be restarted. Closing...\n");
      return SDL_APP_SUCCESS;
    }

    if (reloadApp) {
      compileApp(app);
      loadAppLib(app);
    }
  }
  return SDL_APP_CONTINUE;
}

static SDL_AppResult AppLoop(App* app)
{
  app->frameArena.clearAndReinit();
  auto* renderer = app->rendererData.renderer;
  if (auto result = UpdateHotreload(app); result != SDL_APP_CONTINUE) {
    return result;
  }

  if (SDL_GetTicks() > 3000) {
    return SDL_APP_SUCCESS;
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  if (!app->compileError && app->DrawUI) {
    app->DrawUI(app);
  } else {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
  }

  SDL_RenderPresent(renderer);
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

  if (!SDL_CreateWindowAndRenderer("examples/pen/drawing-lines",
          DEFAULT_WINDOW_SIZE.x,
          DEFAULT_WINDOW_SIZE.y,
          0,
          &app->window,
          &app->rendererData.renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetWindowResizable(app->window, true);

  if (!TTF_Init()) {
    return SDL_APP_FAILURE;
  }

  app->rendererData.textEngine = TTF_CreateRendererTextEngine(app->rendererData.renderer);
  if (!app->rendererData.textEngine) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create text engine from renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  StackArena<1024> arena;
  List<int> sizes;
  sizes.push(arena, 12);
  sizes.push(arena, 14);
  sizes.push(arena, 16);
  sizes.push(arena, 18);
  sizes.push(arena, 20);
  sizes.push(arena, 24);
  for (auto size : sizes) {
    TTF_Font* font = TTF_OpenFont("resource/Roboto-Regular.ttf", size);
    if (!font) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }
    app->rendererData.fonts.push(app->persistentApplicationArena, { font, size });
  }

  int w, h;
  SDL_GetRenderOutputSize(app->rendererData.renderer, &w, &h);
  SDL_SetRenderDrawColor(app->rendererData.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(app->rendererData.renderer);
  SDL_SetRenderTarget(app->rendererData.renderer, NULL);
  SDL_SetRenderDrawBlendMode(app->rendererData.renderer, SDL_BLENDMODE_BLEND);

  compileApp(app);
  if (app->compileError) {
    return SDL_APP_CONTINUE;
  }

  loadAppLib(app);

  return SDL_APP_CONTINUE;
}

static void DestroyApp(App* app)
{
  if (app->DestroyApp) {
    app->DestroyApp(app);
  }

  for (size_t i = 0; i < app->rendererData.fonts.length; i++) {
    TTF_CloseFont(app->rendererData.fonts[i].first);
  }

  if (app->rendererData.textEngine) {
    TTF_DestroyRendererTextEngine(app->rendererData.textEngine);
  }

  if (app->mainDocumentRenderTexture) {
    SDL_DestroyTexture(app->mainDocumentRenderTexture);
  }

  if (app->rendererData.renderer) {
    SDL_DestroyRenderer(app->rendererData.renderer);
  }

  if (app->window) {
    SDL_DestroyWindow(app->window);
  }

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
