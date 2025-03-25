
#include "../shared/app.h"
#include "components.cpp"
#include <SDL3/SDL_timer.h>

void ui(App* app)
{
  static uint64_t oldTime = SDL_GetTicksNS();
  uint64_t now = SDL_GetTicksNS();
  uint64_t delta = now - oldTime;
  oldTime = now;

  double alpha = 0.01;
  static double fps = 0;
  double newFPS = 1000000000.0 / delta;
  fps = fps * (1 - alpha) + newFPS * alpha;

  // div(app, "w-full bg-white h-full col"_s, [&](App* app) {
  //   div(app, "bg-[#333] w-full h-[50px]"_s, [&](App* app) { });
  //   div(app, "w-full h-full"_s, [&](App* app) {
  //     div(app, "h-full bg-[#333] w-[200px] text-white"_s,
  //         [&](App* app) { text(app, ""_s, format(app->frameArena, "FPS: {}", fps)); });
  //     div(app, "id-editor-viewport h-full w-full"_s,
  //         [](App* app) { div(app, "h-full w-full"_s, [](App* app) { }, app->mainViewportSoftwareTexture); });
  //   });
  // });
  Clay_ElementData viewportSize = Clay_GetElementData(CLAY_ID("editor-viewport"));
  app->mainViewportBB = viewportSize.boundingBox;
}