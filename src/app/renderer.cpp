
#include "../app.h"
#include "../clay.h"
#include "clay_renderer.cpp"
#include "ui.cpp"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_surface.h>

void RenderPage(Appstate* appstate, Page& page)
{
  // Fill the surface red
  SDL_FillSurfaceRect(page.canvas, NULL, SDL_MapSurfaceRGB(page.canvas, 255, 0, 0));

  // Lock surface if needed
  if (SDL_MUSTLOCK(page.canvas)) {
    SDL_LockSurface(page.canvas);
  }

  // Get pixel buffer
  Uint32* pixels = (Uint32*)page.canvas->pixels;
  int pitch = page.canvas->pitch / sizeof(Uint32); // Convert bytes to pixels

  Uint32 black = SDL_MapSurfaceRGB(page.canvas, 0, 0, 0);
  int w = page.canvas->w, h = page.canvas->h;

  // Draw diagonal cross
  for (int i = 0; i < w && i < h; i++) {
    pixels[i * pitch + i] = black; // Main diagonal
    pixels[(h - i - 1) * pitch + i] = black; // Anti-diagonal
  }

  // Unlock if needed
  if (SDL_MUSTLOCK(page.canvas)) {
    SDL_UnlockSurface(page.canvas);
  }
}

void RenderDocuments(Appstate* appstate)
{
  for (auto& document : appstate->documents) {
    int pageWidth = document.pageWidthPixels;
    int pageHeight = pageWidth * 297 / 210;
    int pageYOffset = 0;
    for (auto& page : document.pages) {
      if (page.canvas) {
        SDL_DestroySurface(page.canvas);
      }
      page.canvas = SDL_CreateSurface(pageWidth, pageHeight, SDL_PIXELFORMAT_RGBA32);
      if (!page.canvas) {
        fprintf(stderr, "Failed to create SDL surface for page\n");
        abort();
      }

      RenderPage(appstate, page);

      SDL_Rect destRect = { pageYOffset, 0, page.canvas->w, page.canvas->h };
      SDL_BlitSurface(page.canvas, NULL, appstate->mainDocumentRenderSurface, &destRect);
    }
  }
}

void RenderMainViewport(Appstate* appstate)
{
  if (!appstate->mainDocumentRenderSurface
      || appstate->mainDocumentRenderSurface->w != appstate->mainDocumentRenderSurfaceSize.x
      || appstate->mainDocumentRenderSurface->h != appstate->mainDocumentRenderSurfaceSize.y) {
    auto size = appstate->mainDocumentRenderSurfaceSize;
    if (appstate->mainDocumentRenderSurface) {
      SDL_DestroySurface(appstate->mainDocumentRenderSurface);
    }
    appstate->mainDocumentRenderSurface = SDL_CreateSurface(size.x, size.y, SDL_PIXELFORMAT_RGBA32);
    if (!appstate->mainDocumentRenderSurface) {
      fprintf(stderr, "Failed to create SDL surface for page\n");
      abort();
    }
  }

  auto surface = appstate->mainDocumentRenderSurface;
  SDL_FillSurfaceRect(surface, NULL, SDL_MapSurfaceRGB(surface, 255, 255, 255));
  if (SDL_MUSTLOCK(surface)) {
    SDL_LockSurface(surface);
  }

  appstate->documents.clear();
  appstate->documents.emplace_back();
  appstate->documents.back().pages.emplace_back();
  appstate->documents.back().pages.emplace_back();

  for (auto& document : appstate->documents) {
    if (document.pageWidthPixels == 0) {
      document.pageWidthPixels = 500;
    }
  }

  RenderDocuments(appstate);

  if (SDL_MUSTLOCK(surface)) {
    SDL_UnlockSurface(surface);
  }
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

extern "C" void RenderClay(Appstate* appstate, Clay_RenderCommandArray* renderCommands) { }

// void ApplicationRenderer::BeginFrame() { Battery::Renderer2D::BeginScene(GetInstance().scene.get()); }

// void ApplicationRenderer::EndFrame() { Battery::Renderer2D::EndScene(); }

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
