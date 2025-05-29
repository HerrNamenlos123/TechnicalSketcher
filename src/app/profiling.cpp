

#include "../shared/app.h"

#include <SDL3/SDL_timer.h>
#include <linux/limits.h>

#define FUNC_NAME (__func__)
#define FUNC_SIGNATURE (__PRETTY_FUNCTION__)

struct ProfilerInstance {
  App* app;
  const char* name;
  double startTime;

  ProfilerInstance(App* app, const char* defaultName, const char* customName = NULL)
  {
    if (customName) {
      name = customName;
    } else {
      name = defaultName;
    }
    this->app = app;
    startTime = SDL_GetTicksNS() / 1000000.0;
  }

  ~ProfilerInstance()
  {
    double now = SDL_GetTicksNS() / 1000000.0;
    auto& results = app->currentProfilingResults;
    if (results.numOfResults >= results.results.length()) {
      return;
    }
    results.results[results.numOfResults++] = {
      .scopeName = name,
      .msTaken = now - startTime,
    };
  }

  static void profileFrametime(App* app)
  {
    static double lastTime = SDL_GetTicksNS() / 1000000.0;
    double now = SDL_GetTicksNS() / 1000000.0;
    app->currentProfilingResults.frametimeMs = now - lastTime;
    lastTime = now;
  }
};

#define PROFILE_SCOPE(...) ProfilerInstance __prof(app, FUNC_NAME, ##__VA_ARGS__);