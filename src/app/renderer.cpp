
#include "../shared/app.h"
#include "../shared/clay.h"
#include "clay/clay_renderer.h"
#include "document.cpp"
#include "ui.cpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

const auto PAGE_OUTLINE_COLOR = Color("#888");
const auto APP_BACKGROUND_COLOR = Color("#DDD");
const auto PAGE_GRID_COLOR = Color("#A8C9E3");

double clamp(double v, double a, double b)
{
  if (v < a) {
    return a;
  } else if (v > b) {
    return b;
  } else {
    return v;
  }
}

Vec2 CatmullRomSection(float t, const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3)
{
  float t2 = t * t;
  float t3 = t2 * t;

  return {
    0.5f * ((2 * p1.x) + (-p0.x + p2.x) * t + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3),
    0.5f * ((2 * p1.y) + (-p0.y + p2.y) * t + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3)
  };
}

List<Vec2> MakeSplinePoints(Arena& arena, List<Vec2> points, float tolerance)
{
  List<Vec2> result;
  if (points.length < 2) {
    return points;
  }
  List<Vec2> splinePoints;
  splinePoints.push(arena, points[0] - (points[1] - points[0]));
  for (auto& point : points) {
    splinePoints.push(arena, point);
  }
  splinePoints.push(arena, points[points.length - 1] - (points[points.length - 2] - points[points.length - 1]));
  for (int i = 0; i < points.length - 1; i++) {
    for (float t = 0; t < 1; t += 0.1f) {
      result.push(arena, CatmullRomSection(t, splinePoints[i], splinePoints[i + 1], splinePoints[i + 2], splinePoints[i + 3]));
    }
  }
  result.push(arena, splinePoints[splinePoints.length - 2]);
  if (tolerance == 0) {
    return result;
  }
  return rdp(arena, result, tolerance);
}

void DrawPoint(App* app, Vec2 pos, Color color, int size = 3)
{
  SDL_FRect rect;
  rect.x = pos.x - (size - 1) / 2.f;
  rect.y = pos.y - (size - 1) / 2.f;
  rect.w = size;
  rect.h = size;
  SDL_SetRenderDrawColor(app->rendererData.renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(app->rendererData.renderer, &rect);
}

void DrawLine(App* app, Vec2 from, Vec2 to, Color color)
{
  SDL_SetRenderDrawColor(app->rendererData.renderer, color.r, color.g, color.b, color.a);
  SDL_RenderLine(app->rendererData.renderer, from.x, from.y, to.x, to.y);
}

void constructLineshapeOutline(App* app, Document& document, LineShape& shape)
{
  auto& renderer = app->rendererData.renderer;
  int pageWidthPx = document.pageWidthPercentOfWindow * app->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;
  auto pageProj = Vec2(pageWidthPx / 210.0, pageHeightPx / 297.0);

  if (shape.points.length <= 1) {
    return;
  }

  List<float> thicknessProfile;
  List<Vec2> centerPoints;
  for (auto& point : shape.points) {
    if (centerPoints.length > 0) {
      auto alpha = 0.5;
      Vec2 lastPoint = centerPoints.back();
      Vec2 newPoint = Vec2(lastPoint.x * (1 - alpha) + point.pos.x * alpha, lastPoint.y * (1 - alpha) + point.pos.y * alpha);
      centerPoints.push(app->frameArena, newPoint);
    } else {
      centerPoints.push(app->frameArena, point.pos);
    }
    thicknessProfile.push(app->frameArena, point.thickness);
  }
  centerPoints = rdp(app->frameArena, centerPoints, 0.2);

  List<Vec2>
      centerSplines = MakeSplinePoints(app->frameArena, centerPoints, 0.1);
  for (size_t i = 0; i < centerSplines.length - 1; i++) {
    DrawLine(app, centerSplines[i] * pageProj, centerSplines[i + 1] * pageProj, "#F00");
    DrawPoint(app, centerSplines[i] * pageProj, "#0F0");
  }

  List<Vec2> leftOutline;
  List<Vec2> rightOutline;
  for (size_t i = 0; i < centerSplines.length - 1; i++) {
    auto& pos = centerSplines[i];
    float lengthProgress = (float)i / (centerSplines.length - 1);
    float thickness = thicknessProfile[(size_t)clamp(floor(lengthProgress * thicknessProfile.length), 0, thicknessProfile.length)];
    Vec2 nextPos = centerSplines[i + 1];

    Vec2 posPx = pos * pageProj;
    Vec2 nextPosPx = nextPos * pageProj;

    SDL_FRect rect;
    rect.x = posPx.x;
    rect.y = posPx.y;
    rect.w = 2;
    rect.h = 2;

    Vec2 dirToNextPoint = (nextPos - pos).normalize();
    Vec2 perp = Vec2(-dirToNextPoint.y, dirToNextPoint.x);

    SDL_SetRenderDrawColor(app->rendererData.renderer, 0, 255, 0, 255);
    Vec2 left = pos + perp * thickness / 2;
    Vec2 right = pos - perp * thickness / 2;
    SDL_RenderLine(app->rendererData.renderer, posPx.x, posPx.y, posPx.x + perp.x * 15, posPx.y + perp.y * 15);
    SDL_SetRenderDrawColor(app->rendererData.renderer, 255, 0, 0, 255);

    rect.x = left.x * pageProj.x;
    rect.y = left.y * pageProj.y;
    rect.w = 2;
    rect.h = 2;
    SDL_RenderFillRect(app->rendererData.renderer, &rect);
    leftOutline.push(app->frameArena, left);

    rect.x = right.x * pageProj.x;
    rect.y = right.y * pageProj.y;
    SDL_RenderFillRect(app->rendererData.renderer, &rect);
    rightOutline.push(app->frameArena, right);

    SDL_SetRenderDrawColor(app->rendererData.renderer, 255, 0, 255, 255);
  }

  List<Vec2> spline = MakeSplinePoints(app->frameArena, leftOutline, 0.1);
  for (size_t i = 0; i < spline.length - 1; i++) {
    DrawLine(app, spline[i] * pageProj, spline[i + 1] * pageProj, "#0000FF");
  }

  spline = MakeSplinePoints(app->frameArena, rightOutline, 0.1);
  for (size_t i = 0; i < spline.length - 1; i++) {
    DrawLine(app, spline[i] * pageProj, spline[i + 1] * pageProj, "#0000FF");
  }
}

void RenderShapesOnPage(App* app, Document& document, Page& page)
{
  auto renderer = app->rendererData.renderer;
  int pageWidthPx = document.pageWidthPercentOfWindow * app->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;
  auto pageProj = Vec2(pageWidthPx / 210.0, pageHeightPx / 297.0);

  while (page.shapes.length > 0) {
    page.shapes.pop();
  }
  LineShape shape;
  // shape.points.push(document.arena, (InterpolationPoint) { .pos = Vec2(50, 80), .thickness = 5 });
  // shape.points.push(document.arena, (InterpolationPoint) { .pos = Vec2(80, 100), .thickness = 10 });
  // shape.points.push(document.arena, (InterpolationPoint) { .pos = Vec2(120, 90), .thickness = 5 });
  // shape.points.push(document.arena, (InterpolationPoint) { .pos = Vec2(160, 120), .thickness = 8 });
  // shape.color = Color("#FF0000");
  // page.shapes.push(document.arena, shape);

  // for (auto& shape : page.shapes) {
  //   SDL_SetRenderDrawColor(renderer, shape.color.r, shape.color.g, shape.color.b, shape.color.a);
  //   // constructLineshapeOutline(app, document, shape);
  // }

  shape = document.currentLine;
  SDL_SetRenderDrawColor(renderer, shape.color.r, shape.color.g, shape.color.b, shape.color.a);
  constructLineshapeOutline(app, document, shape);

  // List<Vec2> points;
  // points.push(app->frameArena, Vec2(30, 45));
  // points.push(app->frameArena, Vec2(50, 80));
  // points.push(app->frameArena, Vec2(60, 85));
  // points.push(app->frameArena, Vec2(100, 50));
  // points.push(app->frameArena, Vec2(130, 110));
  // points.push(app->frameArena, Vec2(160, 70));

  // for (auto& point : points) {
  //   DrawPoint(app, point * pageProj, "#FF0000", 11);
  // }

  // List<Vec2> spline = MakeSplinePoints(app->frameArena, points, 0.1);
  // for (size_t i = 0; i < spline.length - 1; i++) {
  //   DrawLine(app, spline[i] * pageProj, spline[i + 1] * pageProj, "#0000FF");
  // }
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
    int w = tsk_max(size.width, 1.f);
    int h = tsk_max(size.height, 1.f);
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
  FontData font = app->rendererData.fonts[0];
  for (size_t i = 0; i < app->rendererData.numberOfFonts; i++) {
    if (app->rendererData.fonts[i].size == config->fontSize) {
      font = app->rendererData.fonts[i];
      break;
    }
  }

  int width, height;

  if (!TTF_GetStringSize(font.font, text.chars, text.length, &width, &height)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s",
        SDL_GetError());
  }

  return Clay_Dimensions { (float)width, (float)height };
}

void HandleClayErrors(Clay_ErrorData errorData)
{
  printf("%s", errorData.errorText.chars);
}

extern "C" void ResyncApp(App* app)
{
  app->clayArena.clearAndReinit();

  uint64_t totalMemorySize = Clay_MinMemorySize();
  char* memory = app->clayArena.allocate<char>(totalMemorySize);
  Clay_Arena clayMemory = Clay_Arena { .capacity = totalMemorySize, .memory = memory };

  int width, height;
  SDL_GetWindowSize(app->window, &width, &height);

  Clay_Initialize(clayMemory, Clay_Dimensions { (float)width, (float)height },
      Clay_ErrorHandler { HandleClayErrors });
  Clay_SetMeasureTextFunction(SDL_MeasureText, app);
}

extern "C" void InitApp(App* app)
{
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

  addDocument(app);
  addPageToDocument(app, app->documents.back());
  addPageToDocument(app, app->documents.back());
  addPageToDocument(app, app->documents.back());

  app->documents.back().position = Vec2(300, 100);
  app->documents.back().pageWidthPercentOfWindow = 70;
}

extern "C" void DestroyApp(App* app)
{
  for (auto& document : app->documents) {
    unloadDocument(app, document);
  }
}

extern "C" SDL_AppResult EventHandler(App* app, SDL_Event* event)
{
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
    break;

  case SDL_EVENT_WINDOW_RESIZED:
    Clay_SetLayoutDimensions(Clay_Dimensions { (float)event->window.data1,
        (float)event->window.data2 });
    break;

  case SDL_EVENT_MOUSE_MOTION:
    Clay_SetPointerState(Clay_Vector2 { event->motion.x, event->motion.y },
        event->motion.state & SDL_BUTTON_LMASK);
    break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    Clay_SetPointerState(Clay_Vector2 { event->button.x, event->button.y },
        event->button.button == SDL_BUTTON_LEFT);
    break;

  case SDL_EVENT_MOUSE_WHEEL:
    Clay_UpdateScrollContainers(
        true, Clay_Vector2 { event->wheel.x, event->wheel.y }, 0.01f);
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
