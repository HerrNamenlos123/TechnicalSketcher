
#include "../app.h"
#include "../clay.h"
#include "clay_renderer.cpp"
#include "document.cpp"
#include "ui.cpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>

const auto PAGE_OUTLINE_COLOR = parseHexcolor("#888");
const auto APP_BACKGROUND_COLOR = parseHexcolor("#DDD");
const auto PAGE_GRID_COLOR = parseHexcolor("#A8C9E3");

void RenderPage(Appstate* appstate, Document& document, Page& page)
{
  auto renderer = appstate->rendererData.renderer;
  int pageWidthPx = document.pageWidthPercentOfWindow / 100.0 * appstate->mainViewportBB.width;
  int pageHeightPx = pageWidthPx * 297 / 210;
  int gridSpacing = 5;

  SDL_SetRenderTarget(renderer, page.canvas);
  SDL_SetRenderDrawColor(
      renderer, document.paperColor.r, document.paperColor.g, document.paperColor.b, document.paperColor.a);
  SDL_RenderFillRect(renderer, NULL);

  SDL_SetRenderDrawColor(renderer, PAGE_GRID_COLOR.r, PAGE_GRID_COLOR.g, PAGE_GRID_COLOR.b, PAGE_GRID_COLOR.a);
  for (int x_mm = 0; x_mm < 210; x_mm += gridSpacing) {
    float x_px = x_mm / 210.0 * pageWidthPx;
    SDL_RenderLine(renderer, x_px, 0, x_px, pageHeightPx);
  }
  for (int y_mm = 0; y_mm < 297; y_mm += gridSpacing) {
    float y_px = y_mm / 297.0 * pageHeightPx;
    SDL_RenderLine(renderer, 0, y_px, pageWidthPx, y_px);
  }

  SDL_SetRenderTarget(renderer, NULL);
}

void RenderDocuments(Appstate* appstate)
{
  if (appstate->mainViewportBB.width == 0 || appstate->mainViewportBB.height == 0) {
    return;
  }
  auto& document = appstate->documents[appstate->selectedDocument];
  int pageWidthPx = document.pageWidthPercentOfWindow * appstate->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;
  int pageXOffset = document.position.x;
  int pageYOffset = document.position.y;
  int pageGapPercentOfHeight = 3;
  for (auto& page : document.pages) {
    if (!page.canvas || page.canvas->w != pageWidthPx || page.canvas->h != pageHeightPx) {
      if (page.canvas) {
        SDL_DestroyTexture(page.canvas);
      }
      page.canvas = SDL_CreateTexture(
          appstate->rendererData.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, pageWidthPx, pageHeightPx);
      if (!page.canvas) {
        fprintf(stderr, "Failed to create SDL texture for page\n");
        abort();
      }
    }

    Vec2 topLeft = Vec2(pageXOffset, pageYOffset);
    Vec2 bottomRight = Vec2(pageXOffset + pageWidthPx, pageYOffset + pageHeightPx);
    auto bb = appstate->mainViewportBB;
    if (bottomRight.x > 0 && bottomRight.y > 0 && topLeft.x < bb.width && topLeft.y < bb.height) {
      RenderPage(appstate, document, page);

      auto renderer = appstate->rendererData.renderer;
      SDL_SetRenderTarget(renderer, appstate->mainDocumentRenderTexture);
      SDL_FRect destRect = { pageXOffset, pageYOffset, page.canvas->w, page.canvas->h };
      SDL_RenderTexture(renderer, page.canvas, NULL, &destRect);
      SDL_SetRenderDrawColor(
          renderer, PAGE_OUTLINE_COLOR.r, PAGE_OUTLINE_COLOR.g, PAGE_OUTLINE_COLOR.b, PAGE_OUTLINE_COLOR.a);
      SDL_FRect outlineRect = { pageXOffset, pageYOffset, page.canvas->w, page.canvas->h };
      SDL_RenderRect(renderer, &outlineRect);
      SDL_SetRenderTarget(renderer, NULL);
    }

    pageYOffset += pageHeightPx + pageHeightPx * pageGapPercentOfHeight / 100;
  }
}

void RenderMainViewport(Appstate* appstate)
{
  if (!appstate->mainDocumentRenderTexture || appstate->mainDocumentRenderTexture->w != appstate->mainViewportBB.width
      || appstate->mainDocumentRenderTexture->h != appstate->mainViewportBB.height) {
    auto size = appstate->mainViewportBB;
    if (appstate->mainDocumentRenderTexture) {
      SDL_DestroyTexture(appstate->mainDocumentRenderTexture);
    }
    int w = std::max(size.width, 1.f);
    int h = std::max(size.height, 1.f);
    appstate->mainDocumentRenderTexture
        = SDL_CreateTexture(appstate->rendererData.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!appstate->mainDocumentRenderTexture) {
      fprintf(stderr, "Failed to create SDL texture for viewport\n");
      abort();
    }
  }

  auto renderer = appstate->rendererData.renderer;
  auto texture = appstate->mainDocumentRenderTexture;
  SDL_SetRenderTarget(renderer, texture);
  SDL_SetRenderDrawColor(
      renderer, APP_BACKGROUND_COLOR.r, APP_BACKGROUND_COLOR.g, APP_BACKGROUND_COLOR.b, APP_BACKGROUND_COLOR.a);
  SDL_RenderClear(renderer);

  RenderDocuments(appstate);
  SDL_SetRenderTarget(renderer, NULL);
}

static inline Clay_Dimensions SDL_MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, void* _appstate)
{
  Appstate* appstate = (Appstate*)_appstate;
  auto& fonts = appstate->rendererData.fonts;
  TTF_Font* font = std::get<TTF_Font*>(fonts[0]);
  for (auto [_font, size] : fonts) {
    if (size == config->fontSize) {
      font = _font;
    }
  }
  int width, height;

  if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s", SDL_GetError());
  }

  return (Clay_Dimensions) { (float)width, (float)height };
}

void HandleClayErrors(Clay_ErrorData errorData) { printf("%s", errorData.errorText.chars); }

extern "C" void InitClay(Appstate* appstate)
{
  uint64_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = (Clay_Arena) { .capacity = totalMemorySize, .memory = (char*)SDL_malloc(totalMemorySize) };

  int width, height;
  SDL_GetWindowSize(appstate->window, &width, &height);

  Clay_Initialize(
      clayMemory, (Clay_Dimensions) { (float)width, (float)height }, (Clay_ErrorHandler) { HandleClayErrors });
  Clay_SetMeasureTextFunction(SDL_MeasureText, appstate);

  COLORS["white"] = (Clay_Color) { 255, 255, 255, 255 };
  COLORS["black"] = (Clay_Color) { 0, 0, 0, 255 };
  COLORS["red"] = (Clay_Color) { 255, 0, 0, 255 };
  COLORS["green"] = (Clay_Color) { 0, 255, 0, 255 };
  COLORS["blue"] = (Clay_Color) { 0, 0, 255, 255 };
  FONT_SIZES["xs"] = 12;
  FONT_SIZES["sm"] = 14;
  FONT_SIZES["base"] = 16;
  FONT_SIZES["lg"] = 18;
  FONT_SIZES["xl"] = 20;
  FONT_SIZES["2xl"] = 24;

  addDocument(appstate);
  addPageToDocument(appstate, appstate->documents.back());
  addPageToDocument(appstate, appstate->documents.back());
  addPageToDocument(appstate, appstate->documents.back());
}

extern "C" SDL_AppResult EventHandler(Appstate* appstate, SDL_Event* event)
{
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
    break;

  case SDL_EVENT_WINDOW_RESIZED:
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)event->window.data1, (float)event->window.data2 });
    break;

  case SDL_EVENT_MOUSE_MOTION:
    Clay_SetPointerState((Clay_Vector2) { event->motion.x, event->motion.y }, event->motion.state & SDL_BUTTON_LMASK);
    break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    Clay_SetPointerState((Clay_Vector2) { event->button.x, event->button.y }, event->button.button == SDL_BUTTON_LEFT);
    break;

  case SDL_EVENT_MOUSE_WHEEL:
    Clay_UpdateScrollContainers(true, (Clay_Vector2) { event->wheel.x, event->wheel.y }, 0.01f);
    break;

  case SDL_EVENT_FINGER_DOWN:
    processFingerDownEvent(appstate, event->tfinger);
    break;

  case SDL_EVENT_FINGER_MOTION:
    processFingerMotionEvent(appstate, event->tfinger);
    break;

  case SDL_EVENT_FINGER_UP:
    processFingerUpEvent(appstate, event->tfinger);
    break;

  case SDL_EVENT_FINGER_CANCELED:
    processFingerCancelledEvent(appstate, event->tfinger);
    break;

  default:
    break;
  }
  return SDL_APP_CONTINUE;
}

extern "C" void DrawUI(Appstate* appstate)
{
  RenderMainViewport(appstate);

  const auto STRING_CACHE_SIZE = 1;
  UICache uiCache = {};
  appstate->uiCache = &uiCache;

  Clay_BeginLayout();

  CLAY({
      .layout = {
                  .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
        .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },})
  {
    ui(appstate);
  }

  Clay_RenderCommandArray renderCommands = Clay_EndLayout();
  SDL_Clay_RenderClayCommands(&appstate->rendererData, &renderCommands);
}

// void ApplicationRenderer::DrawLineWorkspace(const glm::vec2& point1,
//                                             const glm::vec2& point2,
//                                             float thickness,
//                                             const glm::vec4& color,
//                                             float falloff)
// {
//   glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1);
//   glm::vec2 p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
//   float thick = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//   Battery::Renderer2D::DrawLine(p1, p2, std::max(thick, 0.5f), color, falloff);
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

//   glm::vec2 p1 = Battery::MathUtils::MapVector(point1, min, max, { 0, 0 }, { width, height });
//   glm::vec2 p2 = Battery::MathUtils::MapVector(point2, min, max, { 0, 0 }, { width, height });
//   float thick = Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

//   Battery::Renderer2D::DrawLine(p1, p2, thick, color, EXPORT_FALLOFF);
// }

// void ApplicationRenderer::DrawCircleWorkspace(const glm::vec2& center,
//                                               float radius,
//                                               float thickness,
//                                               const glm::vec4& color,
//                                               float falloff)
// {
//   glm::vec2 c = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center);
//   float r = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius);
//   float t = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//   Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 }, falloff);
// }

// void ApplicationRenderer::DrawCircleScreenspace(const glm::vec2& center,
//                                                 float radius,
//                                                 float thickness,
//                                                 const glm::vec4& color,
//                                                 float falloff)
// {
//   Battery::Renderer2D::DrawCircle(center, radius, thickness, color, { 0, 0, 0, 0 }, falloff);
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

//   glm::vec2 c = Battery::MathUtils::MapVector(center, min, max, { 0, 0 }, { width, height });
//   float r = Battery::MathUtils::MapFloat(radius, 0, max.x - min.x, 0, width);
//   float t = Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

//   Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 }, EXPORT_FALLOFF);
// }

// void ApplicationRenderer::DrawArcWorkspace(const glm::vec2& center,
//                                            float radius,
//                                            float startAngle,
//                                            float endAngle,
//                                            float thickness,
//                                            const glm::vec4& color,
//                                            float falloff)
// {
//   glm::vec2 c = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center);
//   float r = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius);
//   float t = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//   Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color, falloff);
// }

// void ApplicationRenderer::DrawArcScreenspace(const glm::vec2& center,
//                                              float radius,
//                                              float startAngle,
//                                              float endAngle,
//                                              float thickness,
//                                              const glm::vec4& color,
//                                              float falloff)
// {
//   Battery::Renderer2D::DrawArc(center, radius, startAngle, endAngle, thickness, color, falloff);
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

//   glm::vec2 c = Battery::MathUtils::MapVector(center, min, max, { 0, 0 }, { width, height });
//   float r = Battery::MathUtils::MapFloat(radius, 0, max.x - min.x, 0, width);
//   float t = Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

//   Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color, EXPORT_FALLOFF);
// }

// void ApplicationRenderer::DrawRectangleWorkspace(const glm::vec2& point1,
//                                                  const glm::vec2& point2,
//                                                  float outlineThickness,
//                                                  const glm::vec4& outlineColor,
//                                                  const glm::vec4& fillColor,
//                                                  float falloff)
// {
//   glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1);
//   glm::vec2 p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
//   Battery::Renderer2D::DrawRectangle(p1, p2, outlineThickness, outlineColor, fillColor, falloff);
// }

// void ApplicationRenderer::DrawRectangleScreenspace(const glm::vec2& point1,
//                                                    const glm::vec2& point2,
//                                                    float outlineThickness,
//                                                    const glm::vec4& outlineColor,
//                                                    const glm::vec4& fillColor,
//                                                    float falloff)
// {
//   Battery::Renderer2D::DrawRectangle(point1, point2, outlineThickness, outlineColor, fillColor, falloff);
// }

// void ApplicationRenderer::DrawSelectionBoxInfillRectangle(const glm::vec2& point1, const glm::vec2& point2)
// {
//   ApplicationRenderer::DrawRectangleWorkspace(
//       point1, point2, 0, { 0, 0, 0, 0 }, GetInstance().selectionBoxFillColor, 0);
// }

// void ApplicationRenderer::DrawSelectionBoxOutlineRectangle(const glm::vec2& point1, const glm::vec2& point2)
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
//   glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(position);
//   ApplicationRenderer::DrawRectangleScreenspace(
//       p1 - glm::vec2(GetInstance().previewPointSize / 2, GetInstance().previewPointSize / 2),
//       p1 + glm::vec2(GetInstance().previewPointSize / 2, GetInstance().previewPointSize / 2),
//       1,
//       { 0, 0, 0, 255 },
//       { 255, 255, 255, 255 },
//       0);
// }

// void ApplicationRenderer::DrawGrid(bool infinite)
// {
//   using namespace Battery;
//   auto nav = Navigator::GetInstance();

//   float thickness = GetInstance().gridLineWidth;
//   float alpha
//       = std::min(Navigator::GetInstance()->scale * GetInstance().gridAlphaFactor + GetInstance().gridAlphaOffset,
//                  GetInstance().gridAlphaMax);
//   glm::vec4 color
//       = glm::vec4(GetInstance().gridLineColor, GetInstance().gridLineColor, GetInstance().gridLineColor, alpha);

//   int w = GetMainWindow().GetWidth();
//   int h = GetMainWindow().GetHeight();

//   float right = w;
//   float left = 0;
//   float top = h;
//   float bottom = 0;

//   if (!infinite) { // Draw an A4-sheet
//     glm::vec2 sheetSize = { 210, 297 };
//     right = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(sheetSize / 2.f).x;
//     left = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize / 2.f).x;
//     top = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(sheetSize / 2.f).y;
//     bottom = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize / 2.f).y;

//     for (float x = nav->panOffset.x + w / 2; x < right; x += nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//     }
//     for (float x = nav->panOffset.x + w / 2 - nav->scale * nav->snapSize; x > left; x -= nav->scale *
//     nav->snapSize)
//     {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//     }
//     for (float y = nav->panOffset.y + h / 2; y < top; y += nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//     }
//     for (float y = nav->panOffset.y + h / 2 - nav->scale * nav->snapSize; y > bottom; y -= nav->scale *
//     nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//     }
//   }
//   else {

//     for (float x = nav->panOffset.x + w / 2; x < right; x += nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//     }
//     for (float x = nav->panOffset.x + w / 2; x > left; x -= nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//     }
//     for (float y = nav->panOffset.y + h / 2; y < top; y += nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//     }
//     for (float y = nav->panOffset.y + h / 2; y > bottom; y -= nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//     }
//   }

//   if (!infinite) { // Draw sheet outline
//     Renderer2D::DrawPrimitiveLine({ left, bottom }, { right, bottom }, thickness * 2, color);
//     Renderer2D::DrawPrimitiveLine({ right, bottom }, { right, top }, thickness * 2, color);
//     Renderer2D::DrawPrimitiveLine({ right, top }, { left, top }, thickness * 2, color);
//     Renderer2D::DrawPrimitiveLine({ left, top }, { left, bottom }, thickness * 2, color);
//   }
// }
