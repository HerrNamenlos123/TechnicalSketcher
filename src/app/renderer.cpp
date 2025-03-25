
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
#include <ctime>
#include <unistd.h>

const auto PAGE_OUTLINE_COLOR = Color("#888");
const auto APP_BACKGROUND_COLOR = Color("#DDD");
const auto PAGE_GRID_COLOR = Color("#A8C9E3");

struct PrimitiveRectangle {
  Vec2 pos;
  Vec2 size;
  Color fillColor;
  size_t zindex = 0;
};

struct PrimitiveLine {
  Vec2 from;
  Vec2 to;
  Color lineColor;
  size_t zindex = 0;
};

struct Renderer {
  Arena& arena;
  List<PrimitiveRectangle> rectangles;
  List<PrimitiveLine> lines;
  size_t nextZIndex;
};

void RenderRect(Renderer& renderer, Vec2 pos, Vec2 size, Color color)
{
  renderer.rectangles.push(renderer.arena,
      PrimitiveRectangle {
          .pos = pos,
          .size = size,
          .fillColor = color,
          .zindex = renderer.nextZIndex++,
      });
}

void RenderLine(Renderer& renderer, Vec2 from, Vec2 to, Color color)
{
  renderer.lines.push(renderer.arena,
      PrimitiveLine {
          .from = from,
          .to = to,
          .lineColor = color,
          .zindex = renderer.nextZIndex++,
      });
}

void RenderRectOutline(Renderer& renderer, Vec2 pos, Vec2 size, Color color)
{
  renderer.lines.push(renderer.arena,
      PrimitiveLine {
          .from = pos,
          .to = pos + Vec2(size.x, 0),
          .lineColor = color,
          .zindex = renderer.nextZIndex++,
      });
  renderer.lines.push(renderer.arena,
      PrimitiveLine {
          .from = pos + Vec2(size.x, 0),
          .to = pos + Vec2(size.x, size.y),
          .lineColor = color,
          .zindex = renderer.nextZIndex++,
      });
  renderer.lines.push(renderer.arena,
      PrimitiveLine {
          .from = pos + Vec2(size.x, size.y),
          .to = pos + Vec2(0, size.y),
          .lineColor = color,
          .zindex = renderer.nextZIndex++,
      });
  renderer.lines.push(renderer.arena,
      PrimitiveLine {
          .from = pos + Vec2(0, size.y),
          .to = pos,
          .lineColor = color,
          .zindex = renderer.nextZIndex++,
      });
}

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

  return { 0.5f
        * ((2 * p1.x) + (-p0.x + p2.x) * t + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2
            + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3),
    0.5f
        * ((2 * p1.y) + (-p0.y + p2.y) * t + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2
            + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3) };
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
      result.push(
          arena, CatmullRomSection(t, splinePoints[i], splinePoints[i + 1], splinePoints[i + 2], splinePoints[i + 3]));
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
  // SDL_SetRenderDrawColor(app->rendererData.renderer, color.r, color.g, color.b, color.a);
  // SDL_RenderFillRect(app->rendererData.renderer, &rect);
}

void DrawLine(App* app, Vec2 from, Vec2 to, Color color)
{
  // SDL_SetRenderDrawColor(app->rendererData.renderer, color.r, color.g, color.b, color.a);
  // SDL_RenderLine(app->rendererData.renderer, from.x, from.y, to.x, to.y);
}

void constructLineshapeOutline(App* app, Document& document, LineShape& shape)
{
  // auto& renderer = app->rendererData.renderer;
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
      Vec2 newPoint
          = Vec2(lastPoint.x * (1 - alpha) + point.pos.x * alpha, lastPoint.y * (1 - alpha) + point.pos.y * alpha);
      centerPoints.push(app->frameArena, newPoint);
    } else {
      centerPoints.push(app->frameArena, point.pos);
    }
    thicknessProfile.push(app->frameArena, point.thickness);
  }
  centerPoints = rdp(app->frameArena, centerPoints, 0.2);

  List<Vec2> centerSplines = MakeSplinePoints(app->frameArena, centerPoints, 0.1);
  for (size_t i = 0; i < centerSplines.length - 1; i++) {
    DrawLine(app, centerSplines[i] * pageProj, centerSplines[i + 1] * pageProj, "#F00");
    DrawPoint(app, centerSplines[i] * pageProj, "#0F0");
  }

  List<Vec2> leftOutline;
  List<Vec2> rightOutline;
  for (size_t i = 0; i < centerSplines.length - 1; i++) {
    auto& pos = centerSplines[i];
    float lengthProgress = (float)i / (centerSplines.length - 1);
    float thickness
        = thicknessProfile[(size_t)clamp(floor(lengthProgress * thicknessProfile.length), 0, thicknessProfile.length)];
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

    // SDL_SetRenderDrawColor(app->rendererData.renderer, 0, 255, 0, 255);
    // Vec2 left = pos + perp * thickness / 2;
    // Vec2 right = pos - perp * thickness / 2;
    // SDL_RenderLine(app->rendererData.renderer, posPx.x, posPx.y, posPx.x + perp.x * 15, posPx.y + perp.y * 15);
    // SDL_SetRenderDrawColor(app->rendererData.renderer, 255, 0, 0, 255);

    // rect.x = left.x * pageProj.x;
    // rect.y = left.y * pageProj.y;
    // rect.w = 2;
    // rect.h = 2;
    // SDL_RenderFillRect(app->rendererData.renderer, &rect);
    // leftOutline.push(app->frameArena, left);

    // rect.x = right.x * pageProj.x;
    // rect.y = right.y * pageProj.y;
    // SDL_RenderFillRect(app->rendererData.renderer, &rect);
    // rightOutline.push(app->frameArena, right);

    // SDL_SetRenderDrawColor(app->rendererData.renderer, 255, 0, 255, 255);
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
  // auto renderer = app->rendererData.renderer;
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
  // SDL_SetRenderDrawColor(renderer, shape.color.r, shape.color.g, shape.color.b, shape.color.a);

  if (shape.points.length == 0) {
    return;
  }

  Vec2 topLeft = shape.points[0].pos;
  Vec2 bottomRight = shape.points[0].pos;

  for (auto& p : shape.points) {
    if (p.pos.x - p.thickness / 2.f < topLeft.x) {
      topLeft.x = p.pos.x - p.thickness / 2.f;
    }
    if (p.pos.x + p.thickness / 2.f > bottomRight.x) {
      bottomRight.x = p.pos.x + p.thickness / 2.f;
    }
    if (p.pos.y - p.thickness / 2.f < topLeft.y) {
      topLeft.y = p.pos.y - p.thickness / 2.f;
    }
    if (p.pos.y + p.thickness / 2.f > bottomRight.y) {
      bottomRight.y = p.pos.y + p.thickness / 2.f;
    }
  }

  constructLineshapeOutline(app, document, shape);

  SDL_FRect rect {
    .x = topLeft.x * pageProj.x,
    .y = topLeft.y * pageProj.y,
    .w = bottomRight.x * pageProj.x - topLeft.x * pageProj.x,
    .h = bottomRight.y * pageProj.y - topLeft.y * pageProj.y,
  };
  // SDL_RenderRect(app->rendererData.renderer, &rect);

  Vec2 bbSize = bottomRight - topLeft;
  Vec2 bbSizePx = bbSize * pageProj;
}

void RenderDocuments(App* app, Renderer& renderer)
{
  if (app->mainViewportBB.width == 0 || app->mainViewportBB.height == 0) {
    return;
  }
  auto& document = app->documents[app->selectedDocument];
  int pageWidthPx = document.pageWidthPercentOfWindow * app->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;
  int pageXOffset = document.position.x;
  int pageYOffset = document.position.y;
  int gridSpacing = 5;
  for (auto& page : document.pages) {
    // if (!app->pageRenderTarget || app->pageRenderTarget->w != pageWidthPx || app->pageRenderTarget->h !=
    // pageHeightPx) {
    //   print("");
    //   if (app->pageRenderTarget) {
    //     SDL_DestroyTexture(app->pageRenderTarget);
    //     print("Delete texture");
    //   }
    //   app->pageRenderTarget = SDL_CreateTexture(
    //       app->rendererData.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, pageWidthPx, pageHeightPx);
    //   print("Create texture {} {}", pageWidthPx, pageHeightPx);
    //   if (!app->pageRenderTarget) {
    //     panic("Failed to create SDL texture for page");
    //   }
    // }
    // if (!app->pageSoftwareTexture || app->pageSoftwareTexture->w != pageWidthPx
    //     || app->pageSoftwareTexture->h != pageHeightPx) {
    //   if (app->pageSoftwareTexture) {
    //     SDL_DestroyTexture(app->pageSoftwareTexture);
    //   }
    //   // app->pageSoftwareTexture = SDL_CreateTexture(
    //   //     app->rendererData.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, pageWidthPx,
    //   //     pageHeightPx);
    //   // if (!app->pageSoftwareTexture) {
    //   //   panic("Failed to create SDL software texture for page");
    //   // }
    // }

    Vec2 topLeft = Vec2(pageXOffset, pageYOffset);
    Vec2 bottomRight = Vec2(pageXOffset + pageWidthPx, pageYOffset + pageHeightPx);
    auto bb = app->mainViewportBB;
    if (bottomRight.x > 0 && bottomRight.y > 0 && topLeft.x < bb.width && topLeft.y < bb.height) {
      auto pos = Vec2(pageXOffset, pageYOffset);
      RenderRect(renderer, pos, Vec2(pageWidthPx, pageHeightPx), "#FFF"_s);
      RenderRectOutline(renderer, pos, Vec2(pageWidthPx, pageHeightPx), PAGE_OUTLINE_COLOR);
      for (int x_mm = 0; x_mm < 210; x_mm += gridSpacing) {
        float x_px = x_mm / 210.0 * pageWidthPx;
        RenderLine(renderer, pos + Vec2(x_px, 0), pos + Vec2(x_px, pageHeightPx), PAGE_GRID_COLOR);
      }
      for (int y_mm = 0; y_mm < 297; y_mm += gridSpacing) {
        float y_px = y_mm / 297.0 * pageHeightPx;
        RenderLine(renderer, pos + Vec2(0, y_px), pos + Vec2(pageWidthPx, y_px), PAGE_GRID_COLOR);
      }
    }

    pageYOffset += pageHeightPx + pageHeightPx * app->pageGapPercentOfHeight / 100.f;
  }
}

void RenderMainViewport(App* app)
{
  Renderer renderer = { .arena = app->frameArena };
  renderer.nextZIndex = 1;

  RenderDocuments(app, renderer);

  // Lines
  float* lineVertices = app->frameArena.allocate<float>(renderer.lines.length * 14);
  GLuint* lineIndices = app->frameArena.allocate<GLuint>(renderer.lines.length * 2);
  size_t vertexIndex = 0;
  size_t indexIndex = 0;
  size_t i = 0;
  for (auto& line : renderer.lines) {
    lineVertices[vertexIndex++] = line.from.x;
    lineVertices[vertexIndex++] = line.from.y;
    lineVertices[vertexIndex++] = 1 - (float)line.zindex / renderer.nextZIndex;
    lineVertices[vertexIndex++] = line.lineColor.r / 255.f;
    lineVertices[vertexIndex++] = line.lineColor.g / 255.f;
    lineVertices[vertexIndex++] = line.lineColor.b / 255.f;
    lineVertices[vertexIndex++] = line.lineColor.a / 255.f;
    lineVertices[vertexIndex++] = line.to.x;
    lineVertices[vertexIndex++] = line.to.y;
    lineVertices[vertexIndex++] = 1 - (float)line.zindex / renderer.nextZIndex;
    lineVertices[vertexIndex++] = line.lineColor.r / 255.f;
    lineVertices[vertexIndex++] = line.lineColor.g / 255.f;
    lineVertices[vertexIndex++] = line.lineColor.b / 255.f;
    lineVertices[vertexIndex++] = line.lineColor.a / 255.f;

    lineIndices[indexIndex++] = i * 2;
    lineIndices[indexIndex++] = i * 2 + 1;
    i++;
  }

  glBindVertexArray(app->mainViewportVAO);

  glBindBuffer(GL_ARRAY_BUFFER, app->mainViewportVBO);
  glBufferData(GL_ARRAY_BUFFER, renderer.lines.length * 14 * sizeof(float), lineVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->mainViewportIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer.lines.length * 2 * sizeof(GLuint), lineIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glLineWidth(1.f);
  glDrawElements(GL_LINES, renderer.lines.length * 2, GL_UNSIGNED_INT, (void*)0);

  // Rectangles
  float* rectVertices = app->frameArena.allocate<float>(renderer.rectangles.length * 28);
  GLuint* rectIndices = app->frameArena.allocate<GLuint>(renderer.rectangles.length * 6);
  vertexIndex = 0;
  indexIndex = 0;
  i = 0;
  for (auto& rect : renderer.rectangles) {
    rectVertices[vertexIndex++] = rect.pos.x;
    rectVertices[vertexIndex++] = rect.pos.y;
    rectVertices[vertexIndex++] = 1 - (float)rect.zindex / renderer.nextZIndex;
    rectVertices[vertexIndex++] = rect.fillColor.r / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.g / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.b / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.a / 255.f;
    rectVertices[vertexIndex++] = rect.pos.x + rect.size.x;
    rectVertices[vertexIndex++] = rect.pos.y;
    rectVertices[vertexIndex++] = 1 - (float)rect.zindex / renderer.nextZIndex;
    rectVertices[vertexIndex++] = rect.fillColor.r / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.g / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.b / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.a / 255.f;
    rectVertices[vertexIndex++] = rect.pos.x + rect.size.x;
    rectVertices[vertexIndex++] = rect.pos.y + rect.size.y;
    rectVertices[vertexIndex++] = 1 - (float)rect.zindex / renderer.nextZIndex;
    rectVertices[vertexIndex++] = rect.fillColor.r / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.g / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.b / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.a / 255.f;
    rectVertices[vertexIndex++] = rect.pos.x;
    rectVertices[vertexIndex++] = rect.pos.y + rect.size.y;
    rectVertices[vertexIndex++] = 1 - (float)rect.zindex / renderer.nextZIndex;
    rectVertices[vertexIndex++] = rect.fillColor.r / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.g / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.b / 255.f;
    rectVertices[vertexIndex++] = rect.fillColor.a / 255.f;

    rectIndices[indexIndex++] = i * 4 + 0;
    rectIndices[indexIndex++] = i * 4 + 1;
    rectIndices[indexIndex++] = i * 4 + 2;
    rectIndices[indexIndex++] = i * 4 + 2;
    rectIndices[indexIndex++] = i * 4 + 3;
    rectIndices[indexIndex++] = i * 4 + 0;
    i++;
  }

  glBindVertexArray(app->mainViewportVAO);

  glBindBuffer(GL_ARRAY_BUFFER, app->mainViewportVBO);
  glBufferData(GL_ARRAY_BUFFER, renderer.rectangles.length * 28 * sizeof(float), rectVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->mainViewportIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer.rectangles.length * 6 * sizeof(GLuint), rectIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glDrawElements(GL_TRIANGLES, renderer.rectangles.length * 6, GL_UNSIGNED_INT, (void*)0);

  // if (!app->mainViewportSoftwareTexture || app->mainViewportSoftwareTexture->w != app->mainViewportBB.width
  //     || app->mainViewportSoftwareTexture->h != app->mainViewportBB.height) {
  //   auto size = app->mainViewportBB;
  //   if (app->mainViewportSoftwareTexture) {
  //     SDL_DestroyTexture(app->mainViewportSoftwareTexture);
  //   }
  //   int w = tsk_max(size.width, 1.f);
  //   int h = tsk_max(size.height, 1.f);
  //   // app->mainViewportSoftwareTexture
  //   // = SDL_CreateTexture(app->rendererData.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, w, h);
  //   // if (!app->mainViewportSoftwareTexture) {
  //   //   panic("Failed to create SDL texture for viewport: {} {}", w, h);
  //   // }

  //   app->recreateGlTexture = true;
  // }

  // if (app->recreateGlTexture) {
  //   app->recreateGlTexture = false;
  //   glBindTexture(GL_TEXTURE_2D, app->mainViewportTEX);
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->mainViewportSoftwareTexture->w,
  //   app->mainViewportSoftwareTexture->h,
  //       0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //   glRenderbufferStorage(
  //       GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, app->mainViewportSoftwareTexture->w,
  //       app->mainViewportSoftwareTexture->h);
  //   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, app->mainViewportRBO);
  //   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
  //     SDL_Log("Framebuffer not complete!");
  //     return;
  //   }
  // }

  // GLRenderState renderState = saveRenderState();
  // auto renderer = app->rendererData.renderer;
  // glUseProgram(app->lineshapeShaderprogram);

  // glBindFramebuffer(GL_FRAMEBUFFER, app->mainViewportFBO);
  // glBindTexture(GL_TEXTURE_2D, app->mainViewportTEX);
  // glBindRenderbuffer(GL_RENDERBUFFER, app->mainViewportRBO);

  // glBindVertexArray(app->mainViewportVAO);
  // glBindBuffer(GL_ARRAY_BUFFER, app->mainViewportVBO);
  // glBindRenderbuffer(GL_RENDERBUFFER, app->mainViewportRBO);
  // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, app->mainViewportTEX, 0);

  // float vertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f };

  // glBindVertexArray(app->mainViewportVAO);
  // glBindBuffer(GL_ARRAY_BUFFER, app->mainViewportVBO);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
  // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  // glEnableVertexAttribArray(0);

  // glViewport(0, 0, app->mainViewportSoftwareTexture->w, app->mainViewportSoftwareTexture->h);
  // glClearColor(0, 0, 0, 0.f);
  // glClear(GL_COLOR_BUFFER_BIT);
  // glDrawArrays(GL_TRIANGLES, 0, 3);

  // unsigned char* buf = app->frameArena.allocate<unsigned char>(
  //     app->mainViewportSoftwareTexture->w * app->mainViewportSoftwareTexture->h * 4);
  // glReadPixels(
  //     0, 0, app->mainViewportSoftwareTexture->w, app->mainViewportSoftwareTexture->h, GL_RGBA, GL_UNSIGNED_BYTE,
  //     buf);

  // void* pixels;
  // int pitch;
  // SDL_LockTexture(app->mainViewportSoftwareTexture, NULL, &pixels, &pitch);
  // memcpy(pixels, buf, pitch * app->mainViewportSoftwareTexture->h);
  // SDL_UnlockTexture(app->mainViewportSoftwareTexture);
  // SDL_RenderTexture(renderer, app->mainViewportSoftwareTexture, NULL, NULL);

  // RenderDocuments(app);
  // SDL_SetRenderTarget(renderer, NULL);
  // restoreRenderState(renderState);
}

static inline Clay_Dimensions SDL_MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, void* _app)
{
  App* app = (App*)_app;
  FontData font = app->rendererData.fonts[0];
  for (size_t i = 0; i < app->rendererData.numberOfFonts; i++) {
    if (app->rendererData.fonts[i].size == config->fontSize) {
      font = app->rendererData.fonts[i];
      break;
    }
  }

  int width, height;

  if (!TTF_GetStringSize(font.font, text.chars, text.length, &width, &height)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s", SDL_GetError());
  }

  return Clay_Dimensions { (float)width, (float)height };
}

void HandleClayErrors(Clay_ErrorData errorData)
{
  print("{}", errorData.errorText.chars);
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
