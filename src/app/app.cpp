
#include "renderer.cpp"

#include "shader.cpp"

void setPixelProjection(App* app, float windowWidth, float windowHeight)
{
  Mat4 pixelProjection = Mat4::Identity();
  pixelProjection.applyScaling(1, -1, 1);
  pixelProjection.applyTranslation(-1, -1, 0);
  pixelProjection.applyScaling(2 / windowWidth, 2 / windowHeight, 1);
  setUniformMat4(app->mainShader, "pixelProjection", pixelProjection);
}

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
  Clay_SetMeasureTextFunction(SDL_MeasureText, app);

  if (firstLoad) {
    COLORS.push(app->persistentApplicationArena, { "white"_s, Color(255, 255, 255, 255) });
    COLORS.push(app->persistentApplicationArena, { "black"_s, Color(0, 0, 0, 255) });
    COLORS.push(app->persistentApplicationArena, { "red"_s, Color(255, 0, 0, 255) });
    COLORS.push(app->persistentApplicationArena, { "green"_s, Color(0, 255, 0, 255) });
    COLORS.push(app->persistentApplicationArena, { "blue"_s, Color(0, 0, 255, 255) });
    COLORS.push(app->persistentApplicationArena, { "yellow"_s, Color(255, 255, 0, 255) });
    COLORS.push(app->persistentApplicationArena, { "magenta"_s, Color(255, 0, 255, 255) });
    COLORS.push(app->persistentApplicationArena, { "cyan"_s, Color(0, 255, 255, 255) });
    COLORS.push(app->persistentApplicationArena, { "transparent"_s, Color(255, 255, 255, 255) });
    FONT_SIZES.push(app->persistentApplicationArena, { "xs"_s, 12 });
    FONT_SIZES.push(app->persistentApplicationArena, { "sm"_s, 14 });
    FONT_SIZES.push(app->persistentApplicationArena, { "base"_s, 16 });
    FONT_SIZES.push(app->persistentApplicationArena, { "lg"_s, 18 });
    FONT_SIZES.push(app->persistentApplicationArena, { "xl"_s, 20 });
    FONT_SIZES.push(app->persistentApplicationArena, { "2xl"_s, 24 });
  }

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    SDL_Log("Couldn't load GLAD");
  }
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

  addDocument(app);
  addPageToDocument(app, app->documents.back());
  addPageToDocument(app, app->documents.back());
  addPageToDocument(app, app->documents.back());

  app->documents.back().position = Vec2(300, 100);
  app->documents.back().pageWidthPercentOfWindow = 70;
  app->pageGapPercentOfHeight = 2.f;
}

extern "C" __declspec(dllexport) void UnloadApp(App* app)
{
  for (auto& document : app->documents) {
    unloadDocument(app, document);
  }

  glDeleteVertexArrays(1, &app->mainViewportVAO);
  app->mainViewportVAO = 0;
  glDeleteBuffers(1, &app->mainViewportVBO);
  app->mainViewportVBO = 0;
  glDeleteBuffers(1, &app->mainViewportIBO);
  app->mainViewportIBO = 0;
  glDeleteRenderbuffers(1, &app->mainViewportRBO);
  app->mainViewportRBO = 0;
  glDeleteFramebuffers(1, &app->mainViewportFBO);
  app->mainViewportFBO = 0;
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
    break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    Clay_SetPointerState(Clay_Vector2 { event->button.x, event->button.y }, event->button.button == SDL_BUTTON_LEFT);
    break;

  case SDL_EVENT_MOUSE_WHEEL:
    Clay_UpdateScrollContainers(true, Clay_Vector2 { event->wheel.x, event->wheel.y }, 0.01f);
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

  default:
    break;
  }
  return SDL_APP_CONTINUE;
}

extern "C" __declspec(dllexport) void RenderApp(App* app)
{
  const auto STRING_CACHE_SIZE = 1;
  UICache uiCache = {};
  app->uiCache = &uiCache;

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

  glViewport(0, 0, app->windowSize.x, app->windowSize.y);

  SDL_Clay_RenderClayCommands(&app->rendererData, &renderCommands);

  glViewport(app->mainViewportBB.x, app->mainViewportBB.y, app->mainViewportBB.width, app->mainViewportBB.height);
  glEnable(GL_SCISSOR_TEST);
  glScissor(app->mainViewportBB.x, 0, app->mainViewportBB.width, app->mainViewportBB.height);
  glClearColor(APP_BACKGROUND_COLOR.r / 255.f, APP_BACKGROUND_COLOR.g / 255.f, APP_BACKGROUND_COLOR.b / 255.f,
      APP_BACKGROUND_COLOR.a / 255.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_SCISSOR_TEST);

  RenderMainViewport(app);
}