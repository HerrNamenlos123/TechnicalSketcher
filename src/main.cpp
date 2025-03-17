#include <SDL3_ttf/SDL_ttf.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include "app.h"
#include "clay_renderer.cpp"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_timer.h>
#include <filesystem>
#include <stdio.h>

const Vec2 DEFAULT_WINDOW_SIZE = Vec2(640, 480);
const uint64_t HOTRELOAD_UPDATE_RATE = 100;

bool compileApp(Appstate* appstate)
{
  printf("Application code changed, recompiling...\n");
  auto result = system("bash build-app.sh");
  if (result != 0) {
    printf("Failed to build app!\n");
    appstate->compileError = true;
    return false;
  }

  printf("Done, reloaded app!\n");
  appstate->compileError = false;
  return true;
}

void closeAppLib(Appstate* appstate)
{
  if (appstate->appLibraryHandle) {
    appstate->DrawUI = 0;
    appstate->EventHandler = 0;
    appstate->InitClay = 0;
    dlclose(appstate->appLibraryHandle);
    appstate->appLibraryHandle = 0;
  }
}

bool loadAppLib(Appstate* appstate)
{
  closeAppLib(appstate);

  appstate->appLibraryHandle = dlopen("build/app.so", RTLD_LAZY);
  if (!appstate->appLibraryHandle) {
    printf("Error loading library: %s", dlerror());
    return false;
  }

  appstate->DrawUI = (DrawUI_t)dlsym(appstate->appLibraryHandle, "DrawUI");
  if (appstate->DrawUI == 0) {
    printf("Failed to load func: %s\n", dlerror());
    appstate->compileError = true;
    return false;
  }

  appstate->EventHandler = (EventHandler_t)dlsym(appstate->appLibraryHandle, "EventHandler");
  if (appstate->EventHandler == 0) {
    printf("Failed to load func: %s\n", dlerror());
    appstate->compileError = true;
    return false;
  }

  appstate->InitClay = (InitClay_t)dlsym(appstate->appLibraryHandle, "InitClay");
  if (appstate->InitClay == 0) {
    printf("Failed to load func: %s\n", dlerror());
    appstate->compileError = true;
    return false;
  }

  appstate->InitClay(appstate);

  // Clear any previous errors
  dlerror();
  return true;
}

SDL_AppResult SDL_AppInit(void** _appstate, int argc, char* argv[])
{
  *_appstate = new Appstate();
  if (!*_appstate) {
    return SDL_APP_FAILURE;
  }
  auto appstate = (Appstate*)(*_appstate);
  appstate->lastHotreloadUpdate = SDL_GetTicks();

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

  auto sizes = List<int> { 12, 14, 16, 18, 20, 24 };
  for (auto size : sizes) {
    TTF_Font* font = TTF_OpenFont("resource/Roboto-Regular.ttf", size);
    if (!font) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }
    appstate->rendererData.fonts.push_back(std::make_tuple(font, size));
  }

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

  compileApp(appstate);
  if (!appstate->compileError) {
    loadAppLib(appstate);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* _appstate, SDL_Event* event)
{
  Appstate* appstate = (Appstate*)_appstate;
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }

  if (appstate->EventHandler) {
    return appstate->EventHandler(appstate, event);
  }
  else {
    return SDL_APP_CONTINUE;
  }
}

SDL_AppResult SDL_AppIterate(void* _appstate)
{
  Appstate* appstate = (Appstate*)_appstate;
  auto* renderer = appstate->rendererData.renderer;

  auto now = SDL_GetTicks();
  auto elapsed = now - appstate->lastHotreloadUpdate;
  if (elapsed > HOTRELOAD_UPDATE_RATE) {
    appstate->lastHotreloadUpdate = now;

    bool reloadApp = false;
    for (const auto& entry : fs::directory_iterator(fs::current_path() / "src" / "app")) {
      if (entry.is_regular_file()) {
        auto moddate = fs::last_write_time(entry);
        if (appstate->fileModificationDates.contains(entry.path().string())
            && appstate->fileModificationDates[entry.path().string()] != moddate) {
          reloadApp = true;
        }
        appstate->fileModificationDates[entry.path().string()] = moddate;
      }
    }

    bool reloadCore = false;
    for (const auto& entry : fs::directory_iterator(fs::current_path() / "src")) {
      if (entry.is_regular_file()) {
        auto moddate = fs::last_write_time(entry);
        if (appstate->fileModificationDates.contains(entry.path().string())
            && appstate->fileModificationDates[entry.path().string()] != moddate) {
          reloadCore = true;
        }
        appstate->fileModificationDates[entry.path().string()] = moddate;
      }
    }

    if (reloadCore) {
      printf("Core code changed, application must be restarted. Closing...\n");
      return SDL_APP_SUCCESS;
    }

    if (reloadApp) {
      compileApp(appstate);
      loadAppLib(appstate);
    }
  }

  Clay_RenderCommandArray render_commands;
  if (!appstate->compileError && appstate->DrawUI) {
    render_commands = appstate->DrawUI(appstate);
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  if (appstate->compileError) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
  }

  if (!appstate->compileError && appstate->DrawUI) {
    SDL_Clay_RenderClayCommands(&appstate->rendererData, &render_commands);
  }

  SDL_RenderPresent(renderer);
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
      TTF_CloseFont(std::get<TTF_Font*>(appstate->rendererData.fonts[i]));
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
