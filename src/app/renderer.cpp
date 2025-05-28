
#include "earcut.hpp"
#include <clipper2/clipper.h>

#include "../shared/gl.hpp"
#include "resvg.h"
#include <cairo/cairo.h>

#include "../shared/app.h"
#include "../shared/clay.h"
#include "clay/clay_renderer.h"
#include "document.cpp"
#include "freehand.cpp"
#include "ui.cpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <time.h>
#include <unistd.h>

#include <array>
#include <vector>

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

struct PrimitivePolygon {
  List<Vec2> vertices;
  List<size_t> indices;
  Color color;
  size_t zIndex = 0;
};

struct RenderLineShape {
  LineShape line;
  Vec2 from;
  Vec2 to;
  Color lineColor;
  size_t zindex = 0;
};

struct Renderer {
  Arena& arena;
  App* app;
  List<PrimitiveRectangle> rectangles;
  List<PrimitiveLine> lines;
  List<PrimitivePolygon> polygons;
  size_t nextZIndex;
};

void RenderPolygon(Renderer& renderer, List<Vec2> vertices, List<size_t> indices, Color color)
{
  renderer.polygons.push(renderer.arena,
      PrimitivePolygon {
          .vertices = vertices,
          .indices = indices,
          .color = color,
          .zIndex = renderer.nextZIndex++,
      });
}

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

String getPath(App* app, Arena& arena, List<InterpolationPoint> points)
{
  float penSize_mm = 1;
  auto outline = getStroke(arena, points,
      {
          .size = penSize_mm * app->perfectFreehandAccuracyScaling,
          .thinning = 0,
          .smoothing = 1,
          .streamline = 1,
          .easing =
              [](double t) {
                t--;
                return t * t * t + 1;
              },
          .simulatePressure = false,
          .start = { .cap = true,
              .easing =
                  [](double t) {
                    t--;
                    return t * t * t + 1;
                  },
              },
          .end = { .cap = true,.easing =
                       [](double t) {
                         t--;
                         return t * t * t + 1;
                       } },
      });
  ts::StringBuffer result;
  result.append(arena, "M");
  bool first = true;
  for (auto& p : outline) {
    if (!first) {
      result.append(arena, "L");
    }
    result.append(arena, format(arena, "{} {}", p.x, p.y));
    first = false;
  }
  return result.str();
};

void RenderPage(Renderer& renderer, Document& document, Page& page)
{
  // auto renderer = app->rendererData.renderer;
  int pageWidthPx = document.pageWidthPercentOfWindow * renderer.app->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;
  auto pageProj = Vec2(pageWidthPx / 210.0, pageHeightPx / 297.0);

  if (pageWidthPx == 0 || pageHeightPx == 0) {
    return;
  }

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
  //   // SDL_SetRenderDrawColor(renderer, shape.color.r, shape.color.g, shape.color.b, shape.color.a);
  //   constructLineshapeOutline(renderer.app, document, shape);
  // }
  shape = document.currentLine;
  // constructLineshapeOutline(renderer, document, shape);
  // SDL_SetRenderDrawColor(renderer, shape.color.r, shape.color.g, shape.color.b, shape.color.a);
  // return;

  String svgPath = getPath(renderer.app, renderer.app->frameArena, shape.points);

  ts::StringBuffer svg;
  svg.append(renderer.app->frameArena,
      format(renderer.app->frameArena,
          "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"{}\" height=\"{}\" viewBox=\"0 0 {} {}\">", pageWidthPx,
          pageHeightPx, 210 * renderer.app->perfectFreehandAccuracyScaling,
          297 * renderer.app->perfectFreehandAccuracyScaling));
  svg.append(renderer.app->frameArena, "<path d=\"");
  svg.append(renderer.app->frameArena, svgPath);
  svg.append(renderer.app->frameArena, "\" fill=\"black\" /></svg>");

  resvg_render_tree* tree;
  int err = resvg_parse_tree_from_data(svg.data, svg.length, renderer.app->svgOpts, &tree);
  if (err != RESVG_OK) {
    ts::print_stderr("Error while parsing SVG: {}", err);
    return;
  }

  cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pageWidthPx, pageHeightPx);
  assert(cairo_image_surface_get_stride(surface) == (int)pageWidthPx * 4);

  unsigned char* surface_data = cairo_image_surface_get_data(surface);
  cairo_t* cr = cairo_create(surface);
  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);
  cairo_destroy(cr);

  resvg_render(tree, resvg_transform_identity(), pageWidthPx, pageHeightPx, (char*)surface_data);

  gl::setUniform(renderer.app->mainShader, "uUseTexture", 1.f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  page.renderTexture.uploadData({ pageWidthPx, pageHeightPx }, gl::Format::BGRA, surface_data);

  gl::Vertex quadVertices[4] = {
    {
        .pos = { document.position.x, document.position.y, 0 },
        .color = Color("#0000"),
        .uv = { 0, 0 },
    },
    {
        .pos = { document.position.x + pageWidthPx, document.position.y, 0 },
        .color = Color("#0000"),
        .uv = { 1, 0 },
    },
    {
        .pos = { document.position.x + pageWidthPx, document.position.y + pageHeightPx, 0 },
        .color = Color("#0000"),
        .uv = { 1, 1 },
    },
    {
        .pos = { document.position.x, document.position.y + pageHeightPx, 0 },
        .color = Color("#0000"),
        .uv = { 0, 1 },
    },
  };
  GLuint quadIndices[6] = { 0, 1, 2, 2, 3, 0 };

  glBindVertexArray(renderer.app->mainViewportVAO);
  gl::uploadVertexBufferData(renderer.app->mainViewportVBO, quadVertices, 4, gl::DrawType::Dynamic);
  gl::uploadIndexBufferData(renderer.app->mainViewportIBO, quadIndices, 6, gl::DrawType::Dynamic);
  gl::setupBuffers();

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

  gl::setUniform(renderer.app->mainShader, "uUseTexture", 0.f);

  cairo_surface_destroy(surface);
  resvg_tree_destroy(tree);
  return;

  if (shape.points.length == 0) {
    return;
  }

  Vec2 topLeft = shape.points[0].pos_mm_scaled;
  Vec2 bottomRight = shape.points[0].pos_mm_scaled;

  for (auto& p : shape.points) {
    if (p.pos_mm_scaled.x - p.pressure / 2.f < topLeft.x) {
      topLeft.x = p.pos_mm_scaled.x - p.pressure / 2.f;
    }
    if (p.pos_mm_scaled.x + p.pressure / 2.f > bottomRight.x) {
      bottomRight.x = p.pos_mm_scaled.x + p.pressure / 2.f;
    }
    if (p.pos_mm_scaled.y - p.pressure / 2.f < topLeft.y) {
      topLeft.y = p.pos_mm_scaled.y - p.pressure / 2.f;
    }
    if (p.pos_mm_scaled.y + p.pressure / 2.f > bottomRight.y) {
      bottomRight.y = p.pos_mm_scaled.y + p.pressure / 2.f;
    }
    RenderRect(renderer, p.pos_mm_scaled * pageProj + document.position, Vec2(1, 1), "#F00");
  }

  RenderRectOutline(renderer, topLeft * pageProj + document.position, (bottomRight - topLeft) * pageProj, "#000");

  Vec2 bbSize = bottomRight - topLeft;
  Vec2 bbSizePx = bbSize * pageProj;
}

void RenderDocuments(Renderer& renderer)
{
  if (renderer.app->mainViewportBB.width == 0 || renderer.app->mainViewportBB.height == 0) {
    return;
  }
  auto& document = renderer.app->documents[renderer.app->selectedDocument];
  int pageWidthPx = document.pageWidthPercentOfWindow * renderer.app->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;
  int pageXOffset = document.position.x;
  int pageYOffset = document.position.y;
  int gridSpacing = 5;

  for (auto& page : document.pages) {
    Vec2 topLeft = Vec2(pageXOffset, pageYOffset);
    Vec2 bottomRight = Vec2(pageXOffset + pageWidthPx, pageYOffset + pageHeightPx);
    auto bb = renderer.app->mainViewportBB;
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

    pageYOffset += pageHeightPx + pageHeightPx * renderer.app->pageGapPercentOfHeight / 100.f;
  }
}

void RenderMainViewport(App* app)
{
  Renderer renderer = { .arena = app->frameArena, .app = app };
  renderer.nextZIndex = 1;

  RenderDocuments(renderer);

  // Now draw the line shapes
  auto& doc = app->documents[app->selectedDocument];

  // Lines
  gl::Vertex* lineVertices = app->frameArena.allocate<gl::Vertex>(renderer.lines.length * 2);
  GLuint* lineIndices = app->frameArena.allocate<GLuint>(renderer.lines.length * 2);
  size_t vertexIndex = 0;
  size_t indexIndex = 0;
  size_t i = 0;
  for (auto& line : renderer.lines) {
    lineVertices[vertexIndex++] = {
      .pos = Vec3f(line.from.x, line.from.y, 1 - (float)line.zindex / renderer.nextZIndex),
      .color = line.lineColor / 255,
    };
    lineVertices[vertexIndex++] = {
      .pos = Vec3f(line.to.x, line.to.y, 1 - (float)line.zindex / renderer.nextZIndex),
      .color = line.lineColor / 255,
    };

    lineIndices[indexIndex++] = i * 2;
    lineIndices[indexIndex++] = i * 2 + 1;
    i++;
  }

  glBindVertexArray(app->mainViewportVAO);

  glBindBuffer(GL_ARRAY_BUFFER, app->mainViewportVBO);
  glBufferData(GL_ARRAY_BUFFER, renderer.lines.length * 2 * sizeof(gl::Vertex), lineVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->mainViewportIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer.lines.length * 2 * sizeof(GLuint), lineIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)(sizeof(Vec3f)));
  glEnableVertexAttribArray(1);
  glLineWidth(1.f);
  glDrawElements(GL_LINES, renderer.lines.length * 2, GL_UNSIGNED_INT, (void*)0);

  // Rectangles
  gl::Vertex* rectVertices = app->frameArena.allocate<gl::Vertex>(renderer.rectangles.length * 4);
  GLuint* rectIndices = app->frameArena.allocate<GLuint>(renderer.rectangles.length * 6);
  vertexIndex = 0;
  indexIndex = 0;
  i = 0;
  for (auto& rect : renderer.rectangles) {
    rectVertices[vertexIndex++] = {
      .pos = Vec3f(rect.pos.x, rect.pos.y, 1 - (float)rect.zindex / renderer.nextZIndex),
      .color = rect.fillColor / 255,
    };
    rectVertices[vertexIndex++] = {
      .pos = Vec3f(rect.pos.x + rect.size.x, rect.pos.y, 1 - (float)rect.zindex / renderer.nextZIndex),
      .color = rect.fillColor / 255,
    };
    rectVertices[vertexIndex++] = {
      .pos = Vec3f(rect.pos.x + rect.size.x, rect.pos.y + rect.size.y, 1 - (float)rect.zindex / renderer.nextZIndex),
      .color = rect.fillColor / 255,
    };
    rectVertices[vertexIndex++] = {
      .pos = Vec3f(rect.pos.x, rect.pos.y + rect.size.y, 1 - (float)rect.zindex / renderer.nextZIndex),
      .color = rect.fillColor / 255,
    };

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
  glBufferData(GL_ARRAY_BUFFER, renderer.rectangles.length * 4 * sizeof(gl::Vertex), rectVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->mainViewportIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer.rectangles.length * 6 * sizeof(GLuint), rectIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)(sizeof(Vec3f)));
  glEnableVertexAttribArray(1);

  glDrawElements(GL_TRIANGLES, renderer.rectangles.length * 6, GL_UNSIGNED_INT, (void*)0);

  // Polygons
  if (renderer.polygons.length > 0) {

    size_t totalPolygonVertices = 0;
    size_t totalPolygonIndices = 0;
    for (auto polygon : renderer.polygons) {
      totalPolygonVertices += polygon.vertices.length;
      totalPolygonIndices += polygon.indices.length;
    }

    gl::Vertex* polygonVertices = app->frameArena.allocate<gl::Vertex>(totalPolygonVertices);
    GLuint* polygonIndices = app->frameArena.allocate<GLuint>(totalPolygonIndices);
    size_t numPolygonVertices = 0;
    size_t numPolygonIndices = 0;
    for (size_t i = 0; i < renderer.polygons.length; i++) {
      auto& polygon = renderer.polygons[i];
      for (size_t vertexNum = 0; vertexNum < polygon.vertices.length; vertexNum++) {
        auto& vertex = polygon.vertices[vertexNum];
        polygonVertices[numPolygonVertices + vertexNum] = (gl::Vertex) {
          .pos = { vertex.x, vertex.y, 1 - (float)polygon.zIndex / renderer.nextZIndex },
          .color = polygon.color / 255,
        };
      }
      for (size_t indexNum = 0; indexNum < polygon.indices.length; indexNum++) {
        polygonIndices[numPolygonIndices + indexNum] = polygon.indices[indexNum] + numPolygonVertices;
      }
      numPolygonVertices += polygon.vertices.length;
      numPolygonIndices += polygon.indices.length;
    }

    glBindVertexArray(app->mainViewportVAO);

    glBindBuffer(GL_ARRAY_BUFFER, app->mainViewportVBO);
    glBufferData(GL_ARRAY_BUFFER, totalPolygonVertices * sizeof(gl::Vertex), polygonVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->mainViewportIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalPolygonIndices * sizeof(GLuint), polygonIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)(sizeof(Vec3f)));
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, totalPolygonIndices, GL_UNSIGNED_INT, (void*)0);
  }

  for (auto page : doc.pages) {
    RenderPage(renderer, doc, page);
  }

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
