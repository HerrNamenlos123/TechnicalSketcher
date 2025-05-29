
#include "renderer.cpp"

#include "shader.cpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pen.h>

extern "C" __declspec(dllexport) void LoadApp(App* app, bool firstLoad)
{
  app->clayArena.clearAndReinit();
  app->frameArena.clearAndReinit();

  int width, height;
  SDL_GetWindowSize(app->window, &width, &height);
  app->windowSize = Vec2(width, height);

  uint64_t totalMemorySize = Clay_MinMemorySize();
  char* memory = app->clayArena.allocate<char>(totalMemorySize);
  Clay_Arena clayMemory = Clay_Arena { .capacity = totalMemorySize, .memory = memory };

  Clay_Initialize(clayMemory, Clay_Dimensions { (float)width, (float)height }, Clay_ErrorHandler { HandleClayErrors });
  Clay_SetMeasureTextFunction(MeasureText, app);

  if (firstLoad) {
    auto an = app->persistentApplicationArena;
    app->colors.push(an, { "white"_s, Color(255, 255, 255, 255) });
    app->colors.push(an, { "black"_s, Color(0, 0, 0, 255) });
    app->colors.push(an, { "red"_s, Color(255, 0, 0, 255) });
    app->colors.push(an, { "green"_s, Color(0, 255, 0, 255) });
    app->colors.push(an, { "blue"_s, Color(0, 0, 255, 255) });
    app->colors.push(an, { "yellow"_s, Color(255, 255, 0, 255) });
    app->colors.push(an, { "magenta"_s, Color(255, 0, 255, 255) });
    app->colors.push(an, { "cyan"_s, Color(0, 255, 255, 255) });
    app->colors.push(an, { "transparent"_s, Color(255, 255, 255, 255) });
    app->fonts.push(an, { "xs"_s, 12 });
    app->fonts.push(an, { "sm"_s, 14 });
    app->fonts.push(an, { "base"_s, 16 });
    app->fonts.push(an, { "lg"_s, 18 });
    app->fonts.push(an, { "xl"_s, 20 });
    app->fonts.push(an, { "2xl"_s, 24 });
  }

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    SDL_Log("Couldn't load GLAD");
  }

  app->rendererData.fontContext = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
  if (app->rendererData.fontContext == NULL) {
    print("Could not create stash.\n");
  }

  app->rendererData.fonts = app->persistentApplicationArena.allocate<int>(1);
  app->rendererData.numberOfFonts = 0;
  // auto roboto = b::embed<"resources/Roboto-Regular.ttf">();
  // auto font
  // = fonsAddFontMem(app->rendererData.fontContext, "Roboto", (unsigned char*)roboto.data(), roboto.length(), 0);
  auto font = fonsAddFont(app->rendererData.fontContext, "RobotoRegular", "resource/Roboto-Regular.ttf");
  // auto font
  //     = fonsAddFont(app->rendererData.fontContext, "JetBrainsMono",
  //     "resources/JetBrainsMonoNerdFontMono-Medium.ttf");
  if (font == FONS_INVALID) {
    ts::panic("Could not add font normal.\n");
  }
  app->rendererData.fonts[app->rendererData.numberOfFonts++] = font;

  glViewport(0, 0, width, height);
  app->mainShader = CreateShaderProgram(mainVertexShaderSrc, mainFragmentShaderSrc);
  app->lineshapeShader = CreateShaderProgram(lineshapeVertexShader, lineshapeFragmentShader);
  glUseProgram(app->mainShader);
  setPixelProjection(app, width, height);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glGenVertexArrays(1, &app->mainViewportVAO);
  glGenBuffers(1, &app->mainViewportVBO);
  glGenBuffers(1, &app->mainViewportIBO);
  glGenVertexArrays(1, &app->rendererData.uiVAO);
  glGenBuffers(1, &app->rendererData.uiVBO);
  glGenBuffers(1, &app->rendererData.uiIBO);

  addDocument(app);
  addEmptyPageToDocument(app, app->documents.back());
  addEmptyPageToDocument(app, app->documents.back());
  addEmptyPageToDocument(app, app->documents.back());

  app->documents.back().position = Vec2(300, 100);
  app->documents.back().pageWidthPercentOfWindow = 70;
  app->pageGapPercentOfHeight = 2.f;
  app->currentlyDrawingOnPage = -1;
  app->perfectFreehandAccuracyScaling = 10;
  app->penPressureScaling = 10;

  resvg_init_log();
  app->svgOpts = resvg_options_create();
  resvg_options_load_system_fonts(app->svgOpts);
  resvg_options_set_stylesheet(app->svgOpts, "svg { fill: white; }");
}

extern "C" __declspec(dllexport) void UnloadApp(App* app)
{
  resvg_options_destroy(app->svgOpts);

  for (auto& document : app->documents) {
    unloadDocument(app, document);
  }

  glDeleteVertexArrays(1, &app->rendererData.uiVAO);
  app->rendererData.uiVAO = 0;
  glDeleteBuffers(1, &app->rendererData.uiVBO);
  app->rendererData.uiVBO = 0;
  glDeleteBuffers(1, &app->rendererData.uiIBO);
  app->rendererData.uiIBO = 0;
  glDeleteVertexArrays(1, &app->mainViewportVAO);
  app->mainViewportVAO = 0;
  glDeleteBuffers(1, &app->mainViewportVBO);
  app->mainViewportVBO = 0;
  glDeleteBuffers(1, &app->mainViewportIBO);
  app->mainViewportIBO = 0;
  glDeleteRenderbuffers(1, &app->mainViewportRBO);
  app->mainViewportRBO = 0;
  glDeleteTextures(1, &app->mainViewportTEX);
  app->mainViewportTEX = 0;
  glDeleteProgram(app->mainShader);
  app->mainShader = 0;
}

extern "C" __declspec(dllexport) SDL_AppResult EventHandler(App* app, SDL_Event* event)
{
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
    break;

  case SDL_EVENT_WINDOW_RESIZED:
    Clay_SetLayoutDimensions(Clay_Dimensions { (float)event->window.data1, (float)event->window.data2 });
    setPixelProjection(app, event->window.data1, event->window.data2);
    app->windowSize.x = event->window.data1;
    app->windowSize.y = event->window.data2;
    break;

  case SDL_EVENT_MOUSE_MOTION:
    Clay_SetPointerState(Clay_Vector2 { event->motion.x, event->motion.y }, event->motion.state & SDL_BUTTON_LMASK);
    event->pmotion.x = event->motion.x;
    event->pmotion.y = event->motion.y;
    event->pmotion.pen_state = SDL_PEN_INPUT_DOWN;
    event->paxis.axis = SDL_PEN_AXIS_PRESSURE;
    event->paxis.value = 0.5;
    processPenAxisEvent(app, event->paxis);
    processPenMotionEvent(app, event->pmotion);
    break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    Clay_SetPointerState(Clay_Vector2 { event->button.x, event->button.y }, event->button.button == SDL_BUTTON_LEFT);
    event->ptouch.x = event->button.x;
    event->ptouch.y = event->button.y;
    processPenDownEvent(app, event->ptouch);
    break;

  case SDL_EVENT_MOUSE_BUTTON_UP:
    event->ptouch.x = event->button.x;
    event->ptouch.y = event->button.y;
    processPenUpEvent(app, event->ptouch);
    break;

  case SDL_EVENT_MOUSE_WHEEL:
    Clay_UpdateScrollContainers(true, Clay_Vector2 { event->wheel.x, event->wheel.y }, 0.01f);
    processMouseWheelEvent(app, event->wheel);
    break;

  case SDL_EVENT_FINGER_DOWN:
    processFingerDownEvent(app, event->tfinger);
    break;

  case SDL_EVENT_FINGER_MOTION:
    processFingerMotionEvent(app, event->tfinger);
    break;

  case SDL_EVENT_FINGER_UP:
    processFingerUpEvent(app, event->tfinger);
    break;

  case SDL_EVENT_FINGER_CANCELED:
    processFingerCancelledEvent(app, event->tfinger);
    break;

  case SDL_EVENT_PEN_AXIS:
    processPenAxisEvent(app, event->paxis);
    break;

  case SDL_EVENT_PEN_DOWN:
    processPenDownEvent(app, event->ptouch);
    break;

  case SDL_EVENT_PEN_UP:
    processPenUpEvent(app, event->ptouch);
    break;

  case SDL_EVENT_PEN_MOTION:
    processPenMotionEvent(app, event->pmotion);
    break;

  case SDL_EVENT_PEN_BUTTON_DOWN:
    processPenButtonDownEvent(app, event->pbutton);
    break;

  case SDL_EVENT_PEN_BUTTON_UP:
    processPenButtonUpEvent(app, event->pbutton);
    break;

  case SDL_EVENT_KEY_DOWN:
    if (event->key.scancode == SDL_SCANCODE_SPACE) {
      saveDocumentToFile(app, app->documents[app->selectedDocument], "output.json");
    }
    break;

  default:
    break;
  }
  return SDL_APP_CONTINUE;
}

void DoRenderWork(App* app)
{
  PROFILE_SCOPE();
  const auto STRING_CACHE_SIZE = 1;
  UICache uiCache = {};
  app->uiCache = &uiCache;
  app->rendererData.windowWidth = app->windowSize.x;
  app->rendererData.windowHeight = app->windowSize.y;
  glViewport(0, 0, app->windowSize.x, app->windowSize.y);

  Clay_BeginLayout();

  CLAY({
      .layout = {
          .sizing = { .width = CLAY_SIZING_GROW(0),
              .height = CLAY_SIZING_GROW(0) },
          .layoutDirection = CLAY_TOP_TO_BOTTOM,
      },
  })
  {
    ui(app);
  }

  Clay_RenderCommandArray renderCommands = Clay_EndLayout();

  SDL_Clay_RenderClayCommands(&app->rendererData, &renderCommands);

  setPixelProjection(app, app->mainViewportBB.width, app->mainViewportBB.height);

  glViewport(app->mainViewportBB.x, app->windowSize.y - app->mainViewportBB.y - app->mainViewportBB.height,
      app->mainViewportBB.width, app->mainViewportBB.height);
  glEnable(GL_SCISSOR_TEST);
  glScissor(app->mainViewportBB.x, 0, app->mainViewportBB.width, app->mainViewportBB.height);
  glClearColor(APP_BACKGROUND_COLOR.r / 255.f, APP_BACKGROUND_COLOR.g / 255.f, APP_BACKGROUND_COLOR.b / 255.f,
      APP_BACKGROUND_COLOR.a / 255.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_SCISSOR_TEST);

  RenderMainViewport(app);
}

extern "C" __declspec(dllexport) void RenderApp(App* app)
{
  ProfilerInstance::profileFrametime(app);
  DoRenderWork(app);

  // Profiling done now
  // Swap
  auto tmp = app->lastFrameProfilingResults;
  app->lastFrameProfilingResults = app->currentProfilingResults;
  app->currentProfilingResults = app->lastFrameProfilingResults;

  // And reinit
  app->currentProfilingResults.numOfResults = 0;
}