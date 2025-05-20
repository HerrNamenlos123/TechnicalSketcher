
#include "../shared/app.h"

using ts::print;

const uint64_t HOTRELOAD_UPDATE_RATE = 100;

bool compileApp(App* app)
{
  print("Application code changed, recompiling...\n");
  auto result = system("cmake --build build --target app");
  if (result != 0) {
    print("Failed to build app!\n");
    app->compileError = true;
    return false;
  }

  print("Done, reloaded app!\n");
  app->compileError = false;
  return true;
}

void closeAppLib(App* app)
{
  if (app->appLibraryHandle) {
    app->RenderApp = 0;
    app->EventHandler = 0;
    app->LoadApp = 0;
    app->UnloadApp = 0;
    UnloadLibrary(app->appLibraryHandle);
    app->appLibraryHandle = 0;
  }
}

#define LOAD_FUNC(funcname)                                                                                            \
  if (auto result = LoadLibraryFunc(app->frameArena, app->appLibraryHandle, #funcname##_s)) {                          \
    app->funcname = (funcname##_t) * result;                                                                           \
  } else {                                                                                                             \
    print("Failed to load func: {}", result.error());                                                                  \
    return false;                                                                                                      \
  }

bool loadAppLib(App* app)
{
  closeAppLib(app);

#ifdef TSK_WINDOWS
  String libraryPath = "build/Debug/app.dll"_s;
#else
  String libraryPath = "build/libapp.so"_s;
#endif

  if (auto result = LoadLibrary(app->frameArena, libraryPath)) {
    app->appLibraryHandle = *result;
  } else {
    print("Error loading library: {}", result.error());
    return false;
  }

  LOAD_FUNC(LoadApp);
  LOAD_FUNC(RenderApp);
  LOAD_FUNC(UnloadApp);
  LOAD_FUNC(EventHandler);

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
      auto result = ListDirectory(app->frameArena, format(app->frameArena, "{}/src/app", cwd.value_or(""_s)));
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
            app->fileModificationDates.push(
                app->persistentApplicationArena, { String::clone(app->persistentApplicationArena, file), moddate });
          }
        }
      }
    }

    bool reloadCore = false;
    if (auto cwd = GetCurrentWorkingDirectory(app->frameArena)) {
      auto result = ListDirectory(app->frameArena, format(app->frameArena, "{}/src/core", cwd.value_or(""_s)));
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
            app->fileModificationDates.push(
                app->persistentApplicationArena, { String::clone(app->persistentApplicationArena, file), moddate });
          }
        }
      }
      result = ListDirectory(app->frameArena, format(app->frameArena, "{}/src/shared", cwd.value_or(""_s)));
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
            app->fileModificationDates.push(
                app->persistentApplicationArena, { String::clone(app->persistentApplicationArena, file), moddate });
          }
        }
      }
      result = ListDirectory(app->frameArena, format(app->frameArena, "{}/src/shared/platform", cwd.value_or(""_s)));
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
            app->fileModificationDates.push(
                app->persistentApplicationArena, { String::clone(app->persistentApplicationArena, file), moddate });
          }
        }
      }
    }

    if (reloadCore) {
      print("Core code changed, application must be restarted. Closing...\n");
      return SDL_APP_SUCCESS;
    }

    if (reloadApp) {
      if (compileApp(app)) {
        if (app->UnloadApp) {
          app->UnloadApp(app);
        }
        loadAppLib(app);
        if (app->LoadApp) {
          app->LoadApp(app, false);
        }
      }
    }
  }
  return SDL_APP_CONTINUE;
}