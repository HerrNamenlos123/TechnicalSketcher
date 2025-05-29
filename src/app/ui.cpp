
#include "../shared/app.h"
#include "components.cpp"
#include <SDL3/SDL_timer.h>

static inline Clay_Dimensions MeasureTextImpl(App* app, int fontId, float fontSize, float letterSpacing, String text)
{
  auto& fs = app->rendererData.fontContext;
  int font = app->rendererData.fonts[fontId];

  fonsSetFont(fs, font);
  fonsSetSize(fs, fontSize);
  fonsSetSpacing(fs, letterSpacing);
  fonsSetAlign(fs, FONS_ALIGN_MIDDLE);

  FONStextIter iter;
  FONSquad q;
  float width = 0;

  float ascender, descender, lineh;
  fonsVertMetrics(fs, &ascender, &descender, &lineh);

  fonsTextIterInit(fs, &iter, 0, 0, text.data, text.data + text.length);
  while (fonsTextIterNext(fs, &iter, &q)) {
    width += iter.nextx - iter.x;
  }

  return Clay_Dimensions { width, lineh };
}

static inline Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, void* app)
{
  return MeasureTextImpl(
      (App*)app, config->fontId, config->fontSize, config->letterSpacing, String::view(text.chars, text.length));
}

void ui(App* app)
{
  static uint64_t oldTime = SDL_GetTicksNS();
  uint64_t now = SDL_GetTicksNS();
  uint64_t delta = now - oldTime;
  oldTime = now;

  double alpha = 0.1;
  static double fps = 0;
  double newFPS = 1000000000.0 / delta;
  fps = fps * (1 - alpha) + newFPS * alpha;
  // print("FPS: {}", fps);

  div(app,
      {
          .id = "app-container"_s,
          .width = "full"_s,
          .height = "full"_s,
          .backgroundColor = Color("#FFF"),
          .layoutDirection = "down"_s,
      },
      [&](App* app) {
        div(app,
            {
                .id = "top-bar"_s,
                .width = "full"_s,
                .height = "50px"_s,
                .backgroundColor = Color("#333"),
            },
            [&](App* app) { });
        div(app,
            {
                .id = "sidenav-editor-container"_s,
                .width = "full"_s,
                .height = "full"_s,
            },
            [&](App* app) {
              div(app,
                  {
                      .id = "sidenav"_s,
                      .width = "350px"_s,
                      .height = "full"_s,
                      .backgroundColor = Color("#333"),
                      .fontColor = Color("#FFF"),
                      .layoutDirection = "down"_s,
                  },
                  [&](App* app) {
                    text(app, {}, format(app->frameArena, "FPS: {}", fps));

                    div(app,
                        {
                            .id = "profiling"_s,
                            .layoutDirection = "col"_s,
                        },
                        [&](App* app) {
                          auto& p = app->lastFrameProfilingResults;
                          text(app, {}, "Profiling Results:");

                          text(app, {}, format(app->frameArena, "Frame time: {} ms", p.frametimeMs));

                          for (size_t i = 0; i < p.numOfResults; i++) {
                            auto& r = p.results[i];
                            double ratio = r.msTaken / p.frametimeMs;
                            text(app, {}, format(app->frameArena, "{}: {}%", r.scopeName, ratio * 100));
                          }
                        });
                  });
              div(app,
                  {
                      .id = "editor-viewport"_s,
                      .width = "full"_s,
                      .height = "full"_s,
                  },
                  [](App* app) { });
            });
      });
  Clay_ElementData viewportSize = Clay_GetElementData(CLAY_ID("editor-viewport"));
  app->mainViewportBB = viewportSize.boundingBox;
  // print("{} {}", app->mainViewportBB.width, app->mainViewportBB.height);
}