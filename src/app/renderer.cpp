
#include "../shared/app.h"
#include "../shared/clay.h"
#include "clay_renderer.cpp"
#include "document.cpp"
#include "ui.cpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

const auto PAGE_OUTLINE_COLOR = parseHexcolor("#888"s);
const auto APP_BACKGROUND_COLOR = parseHexcolor("#DDD"s);
const auto PAGE_GRID_COLOR = parseHexcolor("#A8C9E3"s);

void RenderShapesOnPage(App* appstate, Document& document, Page& page)
{
  auto renderer = appstate->rendererData.renderer;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  // for (page.s)
}

void RenderPage(App* appstate, Document& document, Page& page)
{
  auto renderer = appstate->rendererData.renderer;
  int pageWidthPx = document.pageWidthPercentOfWindow / 100.0 * appstate->mainViewportBB.width;
  int pageHeightPx = pageWidthPx * 297 / 210;
  int gridSpacing = 5;

  SDL_SetRenderTarget(renderer, page.canvas);
  SDL_SetRenderDrawColor(renderer, document.paperColor.r, document.paperColor.g,
      document.paperColor.b, document.paperColor.a);
  SDL_RenderFillRect(renderer, NULL);

  SDL_SetRenderDrawColor(renderer, PAGE_GRID_COLOR.r, PAGE_GRID_COLOR.g,
      PAGE_GRID_COLOR.b, PAGE_GRID_COLOR.a);
  for (int x_mm = 0; x_mm < 210; x_mm += gridSpacing) {
    float x_px = x_mm / 210.0 * pageWidthPx;
    SDL_RenderLine(renderer, x_px, 0, x_px, pageHeightPx);
  }
  for (int y_mm = 0; y_mm < 297; y_mm += gridSpacing) {
    float y_px = y_mm / 297.0 * pageHeightPx;
    SDL_RenderLine(renderer, 0, y_px, pageWidthPx, y_px);
  }

  RenderShapesOnPage(appstate, document, page);

  SDL_SetRenderTarget(renderer, NULL);
}

void RenderDocuments(App* app)
{
  if (app->mainViewportBB.width == 0 || app->mainViewportBB.height == 0) {
    return;
  }
  auto& document = app->documents[app->selectedDocument];
  int pageWidthPx = document.pageWidthPercentOfWindow * app->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;
  int pageXOffset = document.position.x;
  int pageYOffset = document.position.y;
  for (auto& page : document.pages) {
    if (!page.canvas || page.canvas->w != pageWidthPx || page.canvas->h != pageHeightPx) {
      if (page.canvas) {
        SDL_DestroyTexture(page.canvas);
      }
      page.canvas = SDL_CreateTexture(
          app->rendererData.renderer, SDL_PIXELFORMAT_RGBA32,
          SDL_TEXTUREACCESS_TARGET, pageWidthPx, pageHeightPx);
      if (!page.canvas) {
        fprintf(stderr, "Failed to create SDL texture for page\n");
        abort();
      }
    }

    Vec2 topLeft = Vec2(pageXOffset, pageYOffset);
    Vec2 bottomRight = Vec2(pageXOffset + pageWidthPx, pageYOffset + pageHeightPx);
    auto bb = app->mainViewportBB;
    if (bottomRight.x > 0 && bottomRight.y > 0 && topLeft.x < bb.width && topLeft.y < bb.height) {
      RenderPage(app, document, page);

      auto renderer = app->rendererData.renderer;
      SDL_SetRenderTarget(renderer, app->mainDocumentRenderTexture);
      SDL_FRect destRect = { pageXOffset, pageYOffset, page.canvas->w,
        page.canvas->h };
      SDL_RenderTexture(renderer, page.canvas, NULL, &destRect);
      SDL_SetRenderDrawColor(renderer, PAGE_OUTLINE_COLOR.r,
          PAGE_OUTLINE_COLOR.g, PAGE_OUTLINE_COLOR.b,
          PAGE_OUTLINE_COLOR.a);
      SDL_FRect outlineRect = { pageXOffset, pageYOffset, page.canvas->w,
        page.canvas->h };
      SDL_RenderRect(renderer, &outlineRect);
      SDL_SetRenderTarget(renderer, NULL);
    }

    pageYOffset += pageHeightPx + pageHeightPx * app->pageGapPercentOfHeight / 100;
  }
}

void RenderMainViewport(App* app)
{
  if (!app->mainDocumentRenderTexture || app->mainDocumentRenderTexture->w != app->mainViewportBB.width || app->mainDocumentRenderTexture->h != app->mainViewportBB.height) {
    auto size = app->mainViewportBB;
    if (app->mainDocumentRenderTexture) {
      SDL_DestroyTexture(app->mainDocumentRenderTexture);
    }
    int w = std::max(size.width, 1.f);
    int h = std::max(size.height, 1.f);
    app->mainDocumentRenderTexture = SDL_CreateTexture(
        app->rendererData.renderer, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_TARGET, w, h);
    if (!app->mainDocumentRenderTexture) {
      fprintf(stderr, "Failed to create SDL texture for viewport\n");
      abort();
    }
  }

  auto renderer = app->rendererData.renderer;
  auto texture = app->mainDocumentRenderTexture;
  SDL_SetRenderTarget(renderer, texture);
  SDL_SetRenderDrawColor(renderer, APP_BACKGROUND_COLOR.r,
      APP_BACKGROUND_COLOR.g, APP_BACKGROUND_COLOR.b,
      APP_BACKGROUND_COLOR.a);
  SDL_RenderClear(renderer);

  RenderDocuments(app);
  SDL_SetRenderTarget(renderer, NULL);
}

static inline Clay_Dimensions SDL_MeasureText(Clay_StringSlice text,
    Clay_TextElementConfig* config,
    void* _app)
{
  App* app = (App*)_app;
  auto& fonts = app->rendererData.fonts;
  TTF_Font* font = fonts[0].first;
  for (auto [_font, size] : fonts) {
    if (size == config->fontSize) {
      font = _font;
    }
  }
  int width, height;

  if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s",
        SDL_GetError());
  }

  return (Clay_Dimensions) { (float)width, (float)height };
}

void HandleClayErrors(Clay_ErrorData errorData)
{
  printf("%s", errorData.errorText.chars);
}

extern "C" void InitClay(App* app)
{
  uint64_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = (Clay_Arena) { .capacity = totalMemorySize,
    .memory = (char*)SDL_malloc(totalMemorySize) };

  int width, height;
  SDL_GetWindowSize(app->window, &width, &height);

  Clay_Initialize(clayMemory, (Clay_Dimensions) { (float)width, (float)height },
      (Clay_ErrorHandler) { HandleClayErrors });
  Clay_SetMeasureTextFunction(SDL_MeasureText, app);

  COLORS.push(app->persistentApplicationArena, { "white"s, Color(255, 255, 255, 255) });
  COLORS.push(app->persistentApplicationArena, { "black"s, Color(0, 0, 0, 255) });
  COLORS.push(app->persistentApplicationArena, { "red"s, Color(255, 0, 0, 255) });
  COLORS.push(app->persistentApplicationArena, { "green"s, Color(0, 255, 0, 255) });
  COLORS.push(app->persistentApplicationArena, { "blue"s, Color(0, 0, 255, 255) });
  COLORS.push(app->persistentApplicationArena, { "yellow"s, Color(255, 255, 0, 255) });
  COLORS.push(app->persistentApplicationArena, { "magenta"s, Color(255, 0, 255, 255) });
  COLORS.push(app->persistentApplicationArena, { "cyan"s, Color(0, 255, 255, 255) });
  COLORS.push(app->persistentApplicationArena, { "transparent"s, Color(255, 255, 255, 255) });
  FONT_SIZES.push(app->persistentApplicationArena, { "xs"s, 12 });
  FONT_SIZES.push(app->persistentApplicationArena, { "sm"s, 14 });
  FONT_SIZES.push(app->persistentApplicationArena, { "base"s, 16 });
  FONT_SIZES.push(app->persistentApplicationArena, { "lg"s, 18 });
  FONT_SIZES.push(app->persistentApplicationArena, { "xl"s, 20 });
  FONT_SIZES.push(app->persistentApplicationArena, { "2xl"s, 24 });

  addDocument(app);
  addPageToDocument(app, app->documents.back());
  addPageToDocument(app, app->documents.back());
  addPageToDocument(app, app->documents.back());

  app->documents.back().position = Vec2(800, 200);
  app->documents.back().pageWidthPercentOfWindow = 30;
}

extern "C" SDL_AppResult EventHandler(App* app, SDL_Event* event)
{
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
    break;

  case SDL_EVENT_WINDOW_RESIZED:
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)event->window.data1,
        (float)event->window.data2 });
    break;

  case SDL_EVENT_MOUSE_MOTION:
    Clay_SetPointerState((Clay_Vector2) { event->motion.x, event->motion.y },
        event->motion.state & SDL_BUTTON_LMASK);
    break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    Clay_SetPointerState((Clay_Vector2) { event->button.x, event->button.y },
        event->button.button == SDL_BUTTON_LEFT);
    break;

  case SDL_EVENT_MOUSE_WHEEL:
    Clay_UpdateScrollContainers(
        true, (Clay_Vector2) { event->wheel.x, event->wheel.y }, 0.01f);
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

extern "C" void DrawUI(App* app)
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

// void ApplicationRenderer::DrawLineWorkspace(const glm::vec2& point1,
//                                             const glm::vec2& point2,
//                                             float thickness,
//                                             const glm::vec4& color,
//                                             float falloff)
// {
//   glm::vec2 p1 =
//   Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1); glm::vec2
//   p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
//   float thick =
//   Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//   Battery::Renderer2D::DrawLine(p1, p2, std::max(thick, 0.5f), color,
//   falloff);
// }

// void ApplicationRenderer::DrawLineScreenspace(const glm::vec2& point1,
//                                               const glm::vec2& point2,
//                                               float thickness,
//                                               const glm::vec4& color,
//                                               float falloff)
// {
//   Battery::Renderer2D::DrawLine(point1, point2, thickness, color, falloff);
// }

// void ApplicationRenderer::DrawLineExport(const glm::vec2& point1,
//                                          const glm::vec2& point2,
//                                          float thickness,
//                                          const glm::vec4& color,
//                                          glm::vec2 min,
//                                          glm::vec2 max,
//                                          float width,
//                                          float height)
// {

//   glm::vec2 p1 = Battery::MathUtils::MapVector(point1, min, max, { 0, 0 }, {
//   width, height }); glm::vec2 p2 = Battery::MathUtils::MapVector(point2, min,
//   max, { 0, 0 }, { width, height }); float thick =
//   Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

//   Battery::Renderer2D::DrawLine(p1, p2, thick, color, EXPORT_FALLOFF);
// }

// void ApplicationRenderer::DrawCircleWorkspace(const glm::vec2& center,
//                                               float radius,
//                                               float thickness,
//                                               const glm::vec4& color,
//                                               float falloff)
// {
//   glm::vec2 c =
//   Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center); float r =
//   Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius); float t
//   = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//   Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 }, falloff);
// }

// void ApplicationRenderer::DrawCircleScreenspace(const glm::vec2& center,
//                                                 float radius,
//                                                 float thickness,
//                                                 const glm::vec4& color,
//                                                 float falloff)
// {
//   Battery::Renderer2D::DrawCircle(center, radius, thickness, color, { 0, 0,
//   0, 0 }, falloff);
// }

// void ApplicationRenderer::DrawCircleExport(const glm::vec2& center,
//                                            float radius,
//                                            float thickness,
//                                            const glm::vec4& color,
//                                            glm::vec2 min,
//                                            glm::vec2 max,
//                                            float width,
//                                            float height)
// {

//   glm::vec2 c = Battery::MathUtils::MapVector(center, min, max, { 0, 0 }, {
//   width, height }); float r = Battery::MathUtils::MapFloat(radius, 0, max.x -
//   min.x, 0, width); float t = Battery::MathUtils::MapFloat(thickness, 0,
//   max.x - min.x, 0, width);

//   Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 },
//   EXPORT_FALLOFF);
// }

// void ApplicationRenderer::DrawArcWorkspace(const glm::vec2& center,
//                                            float radius,
//                                            float startAngle,
//                                            float endAngle,
//                                            float thickness,
//                                            const glm::vec4& color,
//                                            float falloff)
// {
//   glm::vec2 c =
//   Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center); float r =
//   Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius); float t
//   = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//   Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color,
//   falloff);
// }

// void ApplicationRenderer::DrawArcScreenspace(const glm::vec2& center,
//                                              float radius,
//                                              float startAngle,
//                                              float endAngle,
//                                              float thickness,
//                                              const glm::vec4& color,
//                                              float falloff)
// {
//   Battery::Renderer2D::DrawArc(center, radius, startAngle, endAngle,
//   thickness, color, falloff);
// }

// void ApplicationRenderer::DrawArcExport(const glm::vec2& center,
//                                         float radius,
//                                         float startAngle,
//                                         float endAngle,
//                                         float thickness,
//                                         const glm::vec4& color,
//                                         glm::vec2 min,
//                                         glm::vec2 max,
//                                         float width,
//                                         float height)
// {

//   glm::vec2 c = Battery::MathUtils::MapVector(center, min, max, { 0, 0 }, {
//   width, height }); float r = Battery::MathUtils::MapFloat(radius, 0, max.x -
//   min.x, 0, width); float t = Battery::MathUtils::MapFloat(thickness, 0,
//   max.x - min.x, 0, width);

//   Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color,
//   EXPORT_FALLOFF);
// }

// void ApplicationRenderer::DrawRectangleWorkspace(const glm::vec2& point1,
//                                                  const glm::vec2& point2,
//                                                  float outlineThickness,
//                                                  const glm::vec4&
//                                                  outlineColor, const
//                                                  glm::vec4& fillColor, float
//                                                  falloff)
// {
//   glm::vec2 p1 =
//   Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1); glm::vec2
//   p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
//   Battery::Renderer2D::DrawRectangle(p1, p2, outlineThickness, outlineColor,
//   fillColor, falloff);
// }

// void ApplicationRenderer::DrawRectangleScreenspace(const glm::vec2& point1,
//                                                    const glm::vec2& point2,
//                                                    float outlineThickness,
//                                                    const glm::vec4&
//                                                    outlineColor, const
//                                                    glm::vec4& fillColor,
//                                                    float falloff)
// {
//   Battery::Renderer2D::DrawRectangle(point1, point2, outlineThickness,
//   outlineColor, fillColor, falloff);
// }

// void ApplicationRenderer::DrawSelectionBoxInfillRectangle(const glm::vec2&
// point1, const glm::vec2& point2)
// {
//   ApplicationRenderer::DrawRectangleWorkspace(
//       point1, point2, 0, { 0, 0, 0, 0 }, GetInstance().selectionBoxFillColor,
//       0);
// }

// void ApplicationRenderer::DrawSelectionBoxOutlineRectangle(const glm::vec2&
// point1, const glm::vec2& point2)
// {
//   ApplicationRenderer::DrawRectangleWorkspace(point1,
//                                               point2,
//                                               GetInstance().selectionBoxOutlineThickness,
//                                               GetInstance().selectionBoxOutlineColor,
//                                               { 0, 0, 0, 0 },
//                                               0);
// }

// void ApplicationRenderer::DrawPreviewPoint(const glm::vec2& position)
// {
//   glm::vec2 p1 =
//   Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(position);
//   ApplicationRenderer::DrawRectangleScreenspace(
//       p1 - glm::vec2(GetInstance().previewPointSize / 2,
//       GetInstance().previewPointSize / 2), p1 +
//       glm::vec2(GetInstance().previewPointSize / 2,
//       GetInstance().previewPointSize / 2), 1, { 0, 0, 0, 255 }, { 255, 255,
//       255, 255 }, 0);
// }

// void ApplicationRenderer::DrawGrid(bool infinite)
// {
//   using namespace Battery;
//   auto nav = Navigator::GetInstance();

//   float thickness = GetInstance().gridLineWidth;
//   float alpha
//       = std::min(Navigator::GetInstance()->scale *
//       GetInstance().gridAlphaFactor + GetInstance().gridAlphaOffset,
//                  GetInstance().gridAlphaMax);
//   glm::vec4 color
//       = glm::vec4(GetInstance().gridLineColor, GetInstance().gridLineColor,
//       GetInstance().gridLineColor, alpha);

//   int w = GetMainWindow().GetWidth();
//   int h = GetMainWindow().GetHeight();

//   float right = w;
//   float left = 0;
//   float top = h;
//   float bottom = 0;

//   if (!infinite) { // Draw an A4-sheet
//     glm::vec2 sheetSize = { 210, 297 };
//     right =
//     Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(sheetSize
//     / 2.f).x; left =
//     Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize
//     / 2.f).x; top =
//     Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(sheetSize
//     / 2.f).y; bottom =
//     Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize
//     / 2.f).y;

//     for (float x = nav->panOffset.x + w / 2; x < right; x += nav->scale *
//     nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness,
//       color);
//     }
//     for (float x = nav->panOffset.x + w / 2 - nav->scale * nav->snapSize; x >
//     left; x -= nav->scale * nav->snapSize)
//     {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness,
//       color);
//     }
//     for (float y = nav->panOffset.y + h / 2; y < top; y += nav->scale *
//     nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness,
//       color);
//     }
//     for (float y = nav->panOffset.y + h / 2 - nav->scale * nav->snapSize; y >
//     bottom; y -= nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness,
//       color);
//     }
//   }
//   else {

//     for (float x = nav->panOffset.x + w / 2; x < right; x += nav->scale *
//     nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness,
//       color);
//     }
//     for (float x = nav->panOffset.x + w / 2; x > left; x -= nav->scale *
//     nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness,
//       color);
//     }
//     for (float y = nav->panOffset.y + h / 2; y < top; y += nav->scale *
//     nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness,
//       color);
//     }
//     for (float y = nav->panOffset.y + h / 2; y > bottom; y -= nav->scale *
//     nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness,
//       color);
//     }
//   }

//   if (!infinite) { // Draw sheet outline
//     Renderer2D::DrawPrimitiveLine({ left, bottom }, { right, bottom },
//     thickness * 2, color); Renderer2D::DrawPrimitiveLine({ right, bottom }, {
//     right, top }, thickness * 2, color); Renderer2D::DrawPrimitiveLine({
//     right, top }, { left, top }, thickness * 2, color);
//     Renderer2D::DrawPrimitiveLine({ left, top }, { left, bottom }, thickness
//     * 2, color);
//   }
// }
