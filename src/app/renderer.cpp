
#include "../shared/gl.hpp"
#include "resvg.h"
#include <cairo/cairo.h>

#include "../shared/app.h"
#include "../shared/clay.h"
#include "clay/clay_renderer.h"
#include "colors.h"
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

void setUniformMat4(GLuint shader, const char* name, Mat4 matrix)
{
  GLuint matrixLocation = glGetUniformLocation(shader, name);
  if (matrixLocation == -1) {
    print("{}Uniform not found: {}{}", RED, name, RESET);
    return;
  }
  glUniformMatrix4fv(matrixLocation, 1, GL_TRUE, matrix.data.data());
}

void setPixelProjection(App* app, float w, float h)
{
  Mat4 pixelProjection = Mat4::Identity();
  pixelProjection.applyScaling(1, -1, 1);
  pixelProjection.applyTranslation(-1, -1, 0);
  pixelProjection.applyScaling(2, 2, 1);
  pixelProjection.applyScaling(1 / w, 1 / h, 1);
  setUniformMat4(app->mainShader, "pixelProjection", pixelProjection);
}

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

String getPath(App* app, Arena& arena, List<SamplePoint> points)
{
  float penSize_mm = 0.5;
  auto outline = getStroke(arena, points,
      {
          .size = penSize_mm * app->perfectFreehandAccuracyScaling,
          .thinning = 1,
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

void RenderShapeToPageFBO(
    App* app, Renderer& renderer, Document& document, Page& page, LineShape& shape, gl::Framebuffer& fbo)
{
  PROFILE_SCOPE();
  int pageWidthPx = document.pageWidthPercentOfWindow * renderer.app->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;

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
  page.tempRenderTexture.uploadData({ pageWidthPx, pageHeightPx }, gl::Format::BGRA, surface_data);

  gl::Vertex quadVertices[4] = {
    {
        .pos = { 0, 0, 0 },
        .color = Color("#0000"),
        .uv = { 0, 0 },
    },
    {
        .pos = { pageWidthPx, 0, 0 },
        .color = Color("#0000"),
        .uv = { 1, 0 },
    },
    {
        .pos = { pageWidthPx, pageHeightPx, 0 },
        .color = Color("#0000"),
        .uv = { 1, 1 },
    },
    {
        .pos = { 0, pageHeightPx, 0 },
        .color = Color("#0000"),
        .uv = { 0, 1 },
    },
  };
  GLuint quadIndices[6] = { 0, 1, 2, 2, 3, 0 };

  fbo.bind();
  glBindVertexArray(renderer.app->mainViewportVAO);
  gl::uploadVertexBufferData(renderer.app->mainViewportVBO, quadVertices, 4, gl::DrawType::Dynamic);
  gl::uploadIndexBufferData(renderer.app->mainViewportIBO, quadIndices, 6, gl::DrawType::Dynamic);
  gl::setupBuffers();

  setPixelProjection(app, pageWidthPx, pageHeightPx);
  auto& bb = app->mainViewportBB;
  glViewport(0, 0, pageWidthPx, pageHeightPx);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

  fbo.unbind();
  gl::setUniform(renderer.app->mainShader, "uUseTexture", 0.f);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glViewport(app->mainViewportBB.x, app->windowSize.y - app->mainViewportBB.y - app->mainViewportBB.height,
      app->mainViewportBB.width, app->mainViewportBB.height);
  setPixelProjection(app, app->mainViewportBB.width, app->mainViewportBB.height);

  cairo_surface_destroy(surface);
  resvg_tree_destroy(tree);
}

void RenderFBOToPage(App* app, Renderer& renderer, Document& document, Page& page, gl::Framebuffer& fbo)
{
  PROFILE_SCOPE();

  // glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer.app->mainViewportFBO.fbo);
  // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  // glBlitFramebuffer(0, 0, pageWidthPx, pageHeightPx, bb.x + document.position.x,
  //     renderer.app->windowSize.y - (bb.y + document.position.y + pageHeightPx),
  //     bb.x + document.position.x + pageWidthPx, renderer.app->windowSize.y - (bb.y + document.position.y),
  //     GL_COLOR_BUFFER_BIT, GL_NEAREST);

  gl::setUniform(renderer.app->mainShader, "uUseTexture", 1.f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  auto topLeft = page.getTopLeftPx(app);
  auto size = page.getRenderSizePx(app);
  gl::Vertex quadVertices[4] = {
    {
        .pos = { topLeft.x, topLeft.y, 0 },
        .color = Color("#0000"),
        .uv = { 0, 1 },
    },
    {
        .pos = { topLeft.x + size.x, topLeft.y, 0 },
        .color = Color("#0000"),
        .uv = { 1, 1 },
    },
    {
        .pos = { topLeft.x + size.x, topLeft.y + size.y, 0 },
        .color = Color("#0000"),
        .uv = { 1, 0 },
    },
    {
        .pos = { topLeft.x, topLeft.y + size.y, 0 },
        .color = Color("#0000"),
        .uv = { 0, 0 },
    },
  };
  GLuint quadIndices[6] = { 0, 1, 2, 2, 3, 0 };

  glBindVertexArray(renderer.app->mainViewportVAO);
  gl::uploadVertexBufferData(renderer.app->mainViewportVBO, quadVertices, 4, gl::DrawType::Dynamic);
  gl::uploadIndexBufferData(renderer.app->mainViewportIBO, quadIndices, 6, gl::DrawType::Dynamic);
  gl::setupBuffers();

  glBindTexture(GL_TEXTURE_2D, fbo.tex);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
  glBindTexture(GL_TEXTURE_2D, 0);

  gl::setUniform(renderer.app->mainShader, "uUseTexture", 0.f);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void RenderDocumentBackground(App* app, Renderer& renderer)
{
  PROFILE_SCOPE();
  if (renderer.app->mainViewportBB.width == 0 || renderer.app->mainViewportBB.height == 0) {
    return;
  }
  auto& document = renderer.app->documents[renderer.app->selectedDocument];
  int gridSpacing = 5;

  for (auto& page : document.pages) {
    if (!page.overlapsWithViewport(renderer.app)) {
      continue;
    }

    Vec2i pageSizeI = page.getRenderSizePx(renderer.app);
    Vec2 pageSize = { pageSizeI.x, pageSizeI.y };
    Vec2i topLeftI = page.getTopLeftPx(renderer.app);
    Vec2 topLeft = { topLeftI.x, topLeftI.y };
    RenderRect(renderer, topLeft, pageSize, "#FFF"_s);
    RenderRectOutline(renderer, topLeft, pageSize, PAGE_OUTLINE_COLOR);
    for (int x_mm = 0; x_mm < 210; x_mm += gridSpacing) {
      float x_px = x_mm / 210.0 * pageSize.x;
      RenderLine(renderer, topLeft + Vec2(x_px, 0), topLeft + Vec2(x_px, pageSize.y), PAGE_GRID_COLOR);
    }
    for (int y_mm = 0; y_mm < 297; y_mm += gridSpacing) {
      float y_px = y_mm / 297.0 * pageSize.y;
      RenderLine(renderer, topLeft + Vec2(0, y_px), topLeft + Vec2(pageSize.x, y_px), PAGE_GRID_COLOR);
    }
  }
}

void RenderDocumentForeground(App* app, Renderer& renderer)
{
  PROFILE_SCOPE();
  if (renderer.app->mainViewportBB.width == 0 || renderer.app->mainViewportBB.height == 0) {
    return;
  }
  auto& document = renderer.app->documents[renderer.app->selectedDocument];
  int gridSpacing = 5;

  glDisable(GL_DEPTH_TEST);

  for (auto& page : document.pages) {
    if (!page.overlapsWithViewport(renderer.app)) {
      continue;
    }

    auto pageSize = page.getRenderSizePx(renderer.app);
    auto fboSize = page.persistentFBO.getSize();
    if (fboSize.x != pageSize.x || fboSize.y != pageSize.y) {
      page.persistentFBO.clear({ pageSize.x, pageSize.y });
      for (auto& shape : page.shapes) {
        shape.prerendered = false;
        print("Invalidating all prerendered shapes");
      }
    }

    for (auto& shape : page.shapes) {
      if (!shape.prerendered) {
        RenderShapeToPageFBO(app, renderer, document, page, shape, page.persistentFBO);
        shape.prerendered = true;
      }
    }

    RenderFBOToPage(app, renderer, document, page, page.persistentFBO);

    page.previewFBO.clear({ pageSize.x, pageSize.y });
    if (renderer.app->currentlyDrawingOnPage == page.pageNumId) {
      RenderShapeToPageFBO(app, renderer, document, page, document.currentLine, page.previewFBO);
      RenderFBOToPage(app, renderer, document, page, page.previewFBO);
    }
  }

  glEnable(GL_DEPTH_TEST);
}

void RenderMainViewport(App* app)
{
  PROFILE_SCOPE();
  Renderer renderer = { .arena = app->frameArena, .app = app };
  renderer.nextZIndex = 1;

  RenderDocumentBackground(app, renderer);

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
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

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
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

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

  RenderDocumentForeground(app, renderer);
}

void HandleClayErrors(Clay_ErrorData errorData)
{
  print("{}", errorData.errorText.chars);
}
