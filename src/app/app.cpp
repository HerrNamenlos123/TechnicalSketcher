
#include "renderer.cpp"

#include "shader.cpp"

extern "C" __declspec(dllexport) void LoadApp(App* app, bool firstLoad)
{
  app->clayArena.clearAndReinit();
  app->frameArena.clearAndReinit();

  int width, height;
  SDL_GetWindowSize(app->window, &width, &height);

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

  app->lineshapeShaderprogram = CreateShaderProgram();

  float vertices[] = { // Positions       // Colors
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
  };

  glGenVertexArrays(1, &app->mainViewportVAO);
  glGenBuffers(1, &app->mainViewportVBO);
  glBindVertexArray(app->mainViewportVAO);
  glBindBuffer(GL_ARRAY_BUFFER, app->mainViewportVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  addDocument(app);
  addPageToDocument(app, app->documents.back());
  addPageToDocument(app, app->documents.back());
  addPageToDocument(app, app->documents.back());

  app->documents.back().position = Vec2(300, 100);
  app->documents.back().pageWidthPercentOfWindow = 70;
}

extern "C" __declspec(dllexport) void UnloadApp(App* app)
{
  for (auto& document : app->documents) {
    unloadDocument(app, document);
  }

  if (app->mainViewportSoftwareTexture) {
    SDL_DestroyTexture(app->mainViewportSoftwareTexture);
  }

  glDeleteVertexArrays(1, &app->mainViewportVAO);
  app->mainViewportVAO = 0;
  glDeleteBuffers(1, &app->mainViewportVBO);
  app->mainViewportVBO = 0;
  glDeleteProgram(app->lineshapeShaderprogram);
  app->lineshapeShaderprogram = 0;
}

extern "C" __declspec(dllexport) SDL_AppResult EventHandler(App* app, SDL_Event* event)
{
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
    break;

  case SDL_EVENT_WINDOW_RESIZED:
    Clay_SetLayoutDimensions(Clay_Dimensions { (float)event->window.data1, (float)event->window.data2 });
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
  RenderMainViewport(app);

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
  SDL_Clay_RenderClayCommands(&app->rendererData, &renderCommands);
}