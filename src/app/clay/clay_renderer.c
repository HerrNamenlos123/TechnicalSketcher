#include "clay_renderer.h"
#include "../../shared/clay.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "stdio.h"

#define GLFONTSTASH_IMPLEMENTATION
#include "../font/fontstash.h"
#include "../font/glfontstash.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

typedef struct {
  SDL_Renderer* renderer;
  TTF_TextEngine* textEngine;
  TTF_Font** fonts;
} Clay_SDL3RendererData;

/* Global for convenience. Even in 4K this is enough for smooth curves (low radius or rect size coupled with
 * no AA or low resolution might make it appear as jagged curves) */
static int NUM_CIRCLE_SEGMENTS = 16;

// all rendering is performed by a single SDL call, avoiding multiple RenderRect + plumbing choice for circles.
void SDL_Clay_RenderFillRoundedRect(
    Clay_SDL3RendererData* rendererData, const SDL_FRect rect, const float cornerRadius, const Clay_Color _color)
{
  const SDL_FColor color = { _color.r / 255, _color.g / 255, _color.b / 255, _color.a / 255 };

  int indexCount = 0, vertexCount = 0;

  const float minRadius = SDL_min(rect.w, rect.h) / 2.0f;
  const float clampedRadius = SDL_min(cornerRadius, minRadius);

  const int numCircleSegments = SDL_max(NUM_CIRCLE_SEGMENTS, (int)clampedRadius * 0.5f);

  int totalVertices = 4 + (4 * (numCircleSegments * 2)) + 2 * 4;
  int totalIndices = 6 + (4 * (numCircleSegments * 3)) + 6 * 4;

  SDL_Vertex vertices[totalVertices];
  int indices[totalIndices];

  // define center rectangle
  vertices[vertexCount++]
      = (SDL_Vertex) { { rect.x + clampedRadius, rect.y + clampedRadius }, color, { 0, 0 } }; // 0 center TL
  vertices[vertexCount++]
      = (SDL_Vertex) { { rect.x + rect.w - clampedRadius, rect.y + clampedRadius }, color, { 1, 0 } }; // 1 center TR
  vertices[vertexCount++] = (SDL_Vertex) { { rect.x + rect.w - clampedRadius, rect.y + rect.h - clampedRadius }, color,
    { 1, 1 } }; // 2 center BR
  vertices[vertexCount++]
      = (SDL_Vertex) { { rect.x + clampedRadius, rect.y + rect.h - clampedRadius }, color, { 0, 1 } }; // 3 center BL

  indices[indexCount++] = 0;
  indices[indexCount++] = 1;
  indices[indexCount++] = 3;
  indices[indexCount++] = 1;
  indices[indexCount++] = 2;
  indices[indexCount++] = 3;

  // define rounded corners as triangle fans
  const float step = (SDL_PI_F / 2) / numCircleSegments;
  for (int i = 0; i < numCircleSegments; i++) {
    const float angle1 = (float)i * step;
    const float angle2 = ((float)i + 1.0f) * step;

    for (int j = 0; j < 4; j++) { // Iterate over four corners
      float cx, cy, signX, signY;

      switch (j) {
      case 0:
        cx = rect.x + clampedRadius;
        cy = rect.y + clampedRadius;
        signX = -1;
        signY = -1;
        break; // Top-left
      case 1:
        cx = rect.x + rect.w - clampedRadius;
        cy = rect.y + clampedRadius;
        signX = 1;
        signY = -1;
        break; // Top-right
      case 2:
        cx = rect.x + rect.w - clampedRadius;
        cy = rect.y + rect.h - clampedRadius;
        signX = 1;
        signY = 1;
        break; // Bottom-right
      case 3:
        cx = rect.x + clampedRadius;
        cy = rect.y + rect.h - clampedRadius;
        signX = -1;
        signY = 1;
        break; // Bottom-left
      default:
        return;
      }

      vertices[vertexCount++] = (SDL_Vertex) { { cx + SDL_cosf(angle1) * clampedRadius * signX,
                                                   cy + SDL_sinf(angle1) * clampedRadius * signY },
        color, { 0, 0 } };
      vertices[vertexCount++] = (SDL_Vertex) { { cx + SDL_cosf(angle2) * clampedRadius * signX,
                                                   cy + SDL_sinf(angle2) * clampedRadius * signY },
        color, { 0, 0 } };

      indices[indexCount++] = j; // Connect to corresponding central rectangle vertex
      indices[indexCount++] = vertexCount - 2;
      indices[indexCount++] = vertexCount - 1;
    }
  }

  // Define edge rectangles
  //  Top edge
  vertices[vertexCount++] = (SDL_Vertex) { { rect.x + clampedRadius, rect.y }, color, { 0, 0 } }; // TL
  vertices[vertexCount++] = (SDL_Vertex) { { rect.x + rect.w - clampedRadius, rect.y }, color, { 1, 0 } }; // TR

  indices[indexCount++] = 0;
  indices[indexCount++] = vertexCount - 2; // TL
  indices[indexCount++] = vertexCount - 1; // TR
  indices[indexCount++] = 1;
  indices[indexCount++] = 0;
  indices[indexCount++] = vertexCount - 1; // TR
  // Right edge
  vertices[vertexCount++] = (SDL_Vertex) { { rect.x + rect.w, rect.y + clampedRadius }, color, { 1, 0 } }; // RT
  vertices[vertexCount++]
      = (SDL_Vertex) { { rect.x + rect.w, rect.y + rect.h - clampedRadius }, color, { 1, 1 } }; // RB

  indices[indexCount++] = 1;
  indices[indexCount++] = vertexCount - 2; // RT
  indices[indexCount++] = vertexCount - 1; // RB
  indices[indexCount++] = 2;
  indices[indexCount++] = 1;
  indices[indexCount++] = vertexCount - 1; // RB
  // Bottom edge
  vertices[vertexCount++]
      = (SDL_Vertex) { { rect.x + rect.w - clampedRadius, rect.y + rect.h }, color, { 1, 1 } }; // BR
  vertices[vertexCount++] = (SDL_Vertex) { { rect.x + clampedRadius, rect.y + rect.h }, color, { 0, 1 } }; // BL

  indices[indexCount++] = 2;
  indices[indexCount++] = vertexCount - 2; // BR
  indices[indexCount++] = vertexCount - 1; // BL
  indices[indexCount++] = 3;
  indices[indexCount++] = 2;
  indices[indexCount++] = vertexCount - 1; // BL
  // Left edge
  vertices[vertexCount++] = (SDL_Vertex) { { rect.x, rect.y + rect.h - clampedRadius }, color, { 0, 1 } }; // LB
  vertices[vertexCount++] = (SDL_Vertex) { { rect.x, rect.y + clampedRadius }, color, { 0, 0 } }; // LT

  indices[indexCount++] = 3;
  indices[indexCount++] = vertexCount - 2; // LB
  indices[indexCount++] = vertexCount - 1; // LT
  indices[indexCount++] = 0;
  indices[indexCount++] = 3;
  indices[indexCount++] = vertexCount - 1; // LT

  // Render everything
  SDL_RenderGeometry(rendererData->renderer, NULL, vertices, vertexCount, indices, indexCount);
}

static void GL_FillRoundedRect(
    RendererData* rendererData, const SDL_FRect rect, float zIndex, float radius, Clay_Color color)
{
  typedef struct {
    float x, y, z;
  } Pos;
  typedef struct {
    Pos pos;
    Clay_Color color;
  } Vertex;

  Clay_Color c = (Clay_Color) {
    color.r / 255,
    color.g / 255,
    color.b / 255,
    color.a / 255,
  };

  int indexCount = 0, vertexCount = 0;

  const float minRadius = SDL_min(rect.w, rect.h) / 2.0f;
  const float clampedRadius = SDL_min(radius, minRadius);

  const int numCircleSegments = SDL_max(NUM_CIRCLE_SEGMENTS, (int)clampedRadius * 0.5f);

  int totalVertices = 4 + (4 * (numCircleSegments * 2)) + 2 * 4;
  int totalIndices = 6 + (4 * (numCircleSegments * 3)) + 6 * 4;

  Vertex vertices[totalVertices];
  int indices[totalIndices];

  // define center rectangle
  vertices[vertexCount++] = (Vertex) { { rect.x + clampedRadius, rect.y + clampedRadius, zIndex }, c }; // 0 center TL
  vertices[vertexCount++]
      = (Vertex) { { rect.x + rect.w - clampedRadius, rect.y + clampedRadius, zIndex }, c }; // 1 center TR
  vertices[vertexCount++] = (Vertex) {
    { rect.x + rect.w - clampedRadius, rect.y + rect.h - clampedRadius, zIndex },
    c,
  }; // 2 center BR
  vertices[vertexCount++]
      = (Vertex) { { rect.x + clampedRadius, rect.y + rect.h - clampedRadius, zIndex }, c }; // 3 center BL

  indices[indexCount++] = 0;
  indices[indexCount++] = 1;
  indices[indexCount++] = 3;
  indices[indexCount++] = 1;
  indices[indexCount++] = 2;
  indices[indexCount++] = 3;

  // define rounded corners as triangle fans
  const float step = (SDL_PI_F / 2) / numCircleSegments;
  for (int i = 0; i < numCircleSegments; i++) {
    const float angle1 = (float)i * step;
    const float angle2 = ((float)i + 1.0f) * step;

    for (int j = 0; j < 4; j++) { // Iterate over four corners
      float cx, cy, signX, signY;

      switch (j) {
      case 0:
        cx = rect.x + clampedRadius;
        cy = rect.y + clampedRadius;
        signX = -1;
        signY = -1;
        break; // Top-left
      case 1:
        cx = rect.x + rect.w - clampedRadius;
        cy = rect.y + clampedRadius;
        signX = 1;
        signY = -1;
        break; // Top-right
      case 2:
        cx = rect.x + rect.w - clampedRadius;
        cy = rect.y + rect.h - clampedRadius;
        signX = 1;
        signY = 1;
        break; // Bottom-right
      case 3:
        cx = rect.x + clampedRadius;
        cy = rect.y + rect.h - clampedRadius;
        signX = -1;
        signY = 1;
        break; // Bottom-left
      default:
        return;
      }

      vertices[vertexCount++] = (Vertex) {
        { cx + SDL_cosf(angle1) * clampedRadius * signX, cy + SDL_sinf(angle1) * clampedRadius * signY, zIndex },
        c,
      };
      vertices[vertexCount++] = (Vertex) {
        { cx + SDL_cosf(angle2) * clampedRadius * signX, cy + SDL_sinf(angle2) * clampedRadius * signY, zIndex },
        c,
      };

      indices[indexCount++] = j; // Connect to corresponding central rectangle vertex
      indices[indexCount++] = vertexCount - 2;
      indices[indexCount++] = vertexCount - 1;
    }
  }

  // Define edge rectangles
  //  Top edge
  vertices[vertexCount++] = (Vertex) {
    { rect.x + clampedRadius, rect.y, zIndex },
    c,
  }; // TL
  vertices[vertexCount++] = (Vertex) {
    { rect.x + rect.w - clampedRadius, rect.y, zIndex },
    c,
  }; // TR

  indices[indexCount++] = 0;
  indices[indexCount++] = vertexCount - 2; // TL
  indices[indexCount++] = vertexCount - 1; // TR
  indices[indexCount++] = 1;
  indices[indexCount++] = 0;
  indices[indexCount++] = vertexCount - 1; // TR
  // Right edge
  vertices[vertexCount++] = (Vertex) {
    { rect.x + rect.w, rect.y + clampedRadius, zIndex },
    c,
  }; // RT
  vertices[vertexCount++] = (Vertex) {
    { rect.x + rect.w, rect.y + rect.h - clampedRadius, zIndex },
    c,
  }; // RB

  indices[indexCount++] = 1;
  indices[indexCount++] = vertexCount - 2; // RT
  indices[indexCount++] = vertexCount - 1; // RB
  indices[indexCount++] = 2;
  indices[indexCount++] = 1;
  indices[indexCount++] = vertexCount - 1; // RB
  // Bottom edge
  vertices[vertexCount++] = (Vertex) {
    { rect.x + rect.w - clampedRadius, rect.y + rect.h, zIndex },
    c,
  }; // BR
  vertices[vertexCount++] = (Vertex) {
    { rect.x + clampedRadius, rect.y + rect.h, zIndex },
    c,
  }; // BL

  indices[indexCount++] = 2;
  indices[indexCount++] = vertexCount - 2; // BR
  indices[indexCount++] = vertexCount - 1; // BL
  indices[indexCount++] = 3;
  indices[indexCount++] = 2;
  indices[indexCount++] = vertexCount - 1; // BL
  // Left edge
  vertices[vertexCount++] = (Vertex) {
    { rect.x, rect.y + rect.h - clampedRadius, zIndex },
    c,
  }; // LB
  vertices[vertexCount++] = (Vertex) {
    { rect.x, rect.y + clampedRadius, zIndex },
    c,
  }; // LT

  indices[indexCount++] = 3;
  indices[indexCount++] = vertexCount - 2; // LB
  indices[indexCount++] = vertexCount - 1; // LT
  indices[indexCount++] = 0;
  indices[indexCount++] = 3;
  indices[indexCount++] = vertexCount - 1; // LT

  // printf("Rendered rect at %f %f\n", rect.x, rect.y);

  GLint prevVAO, prevArrayBuffer, prevElementBuffer;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevArrayBuffer);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &prevElementBuffer);

  if (color.a != 255.f) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
  }

  glBindVertexArray(rendererData->uiVAO);
  glBindBuffer(GL_ARRAY_BUFFER, rendererData->uiVBO);
  glBufferData(GL_ARRAY_BUFFER, totalVertices * sizeof(vertices[0]), vertices, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererData->uiIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndices * 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glDrawElements(GL_TRIANGLES, totalIndices, GL_UNSIGNED_INT, (void*)0);

  if (color.a != 255.f) {
    glDepthMask(GL_TRUE);
  }
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  glBindVertexArray(prevVAO);
  glBindBuffer(GL_ARRAY_BUFFER, prevArrayBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prevElementBuffer);
}

void SDL_Clay_RenderArc(Clay_SDL3RendererData* rendererData, const SDL_FPoint center, const float radius,
    const float startAngle, const float endAngle, const float thickness, const Clay_Color color)
{
  SDL_SetRenderDrawColor(rendererData->renderer, color.r, color.g, color.b, color.a);

  const float radStart = startAngle * (SDL_PI_F / 180.0f);
  const float radEnd = endAngle * (SDL_PI_F / 180.0f);

  const int numCircleSegments = SDL_max(
      NUM_CIRCLE_SEGMENTS, (int)(radius * 1.5f)); // increase circle segments for larger circles, 1.5 is arbitrary.

  const float angleStep = (radEnd - radStart) / (float)numCircleSegments;
  const float thicknessStep = 0.4f; // arbitrary value to avoid overlapping lines. Changing THICKNESS_STEP or
                                    // numCircleSegments might cause artifacts.

  for (float t = thicknessStep; t < thickness - thicknessStep; t += thicknessStep) {
    SDL_FPoint points[numCircleSegments + 1];
    const float clampedRadius = SDL_max(radius - t, 1.0f);

    for (int i = 0; i <= numCircleSegments; i++) {
      const float angle = radStart + i * angleStep;
      points[i] = (SDL_FPoint) { SDL_roundf(center.x + SDL_cosf(angle) * clampedRadius),
        SDL_roundf(center.y + SDL_sinf(angle) * clampedRadius) };
    }
    SDL_RenderLines(rendererData->renderer, points, numCircleSegments + 1);
  }
}

SDL_Rect currentClippingRectangle;

void SDL_Clay_RenderClayCommands(RendererData* rendererData, Clay_RenderCommandArray* rcommands)
{
  for (size_t i = 0; i < rcommands->length; i++) {
    Clay_RenderCommand* rcmd = Clay_RenderCommandArray_Get(rcommands, i);
    const Clay_BoundingBox bounding_box = rcmd->boundingBox;
    const SDL_FRect rect
        = { (int)bounding_box.x, (int)bounding_box.y, (int)bounding_box.width, (int)bounding_box.height };
    float zIndex = 1 - (float)i / rcommands->length; // built-in zIndex won't work (is always 0)

    switch (rcmd->commandType) {
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
      Clay_RectangleRenderData* config = &rcmd->renderData.rectangle;
      GL_FillRoundedRect(rendererData, rect, zIndex, config->cornerRadius.topLeft, config->backgroundColor);
    } break;
    case CLAY_RENDER_COMMAND_TYPE_TEXT: {
      Clay_TextRenderData* config = &rcmd->renderData.text;
      int font = rendererData->fonts[config->fontId];
      FONScontext* fs = rendererData->fontContext;

      fonsSetSize(fs, config->fontSize);
      fonsSetFont(fs, font);
      fonsSetSpacing(fs, config->letterSpacing);
      fonsSetColor(fs, glfonsRGBA(config->textColor.r, config->textColor.g, config->textColor.b, config->textColor.a));
      fonsSetAlign(fs, FONS_ALIGN_MIDDLE);
      char* text = (char*)config->stringContents.chars;
      size_t len = config->stringContents.length;
      // GL_FillRoundedRect(rendererData, rect, 0, 0, (Clay_Color) { 0, 255, 0, 50 });

      GLint prevProgram;
      glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glUseProgram(0);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_TEXTURE_2D);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, rendererData->windowWidth, rendererData->windowHeight, 0, -1, 1);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glDisable(GL_DEPTH_TEST);
      glColor4ub(255, 255, 255, 255);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_CULL_FACE);

      fonsDrawText(fs, rect.x, rect.y + rect.h / 2, config->stringContents.chars,
          config->stringContents.chars + config->stringContents.length);

      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glPopAttrib();
      glUseProgram(prevProgram);

    } break;
    case CLAY_RENDER_COMMAND_TYPE_BORDER: {
      Clay_BorderRenderData* config = &rcmd->renderData.border;

      const float minRadius = SDL_min(rect.w, rect.h) / 2.0f;
      const Clay_CornerRadius clampedRadii = { .topLeft = SDL_min(config->cornerRadius.topLeft, minRadius),
        .topRight = SDL_min(config->cornerRadius.topRight, minRadius),
        .bottomLeft = SDL_min(config->cornerRadius.bottomLeft, minRadius),
        .bottomRight = SDL_min(config->cornerRadius.bottomRight, minRadius) };
      // edges
      Clay_Color color = { config->color.r, config->color.g, config->color.b, config->color.a };

      if (config->width.left > 0) {
        const float starting_y = rect.y + clampedRadii.topLeft;
        const float length = rect.h - clampedRadii.topLeft - clampedRadii.bottomLeft;
        SDL_FRect line = { rect.x, starting_y, config->width.left, length };
        // SDL_RenderFillRect(rendererData->renderer, &line);
        GL_FillRoundedRect(rendererData, line, 0, zIndex, color);
      }
      if (config->width.right > 0) {
        const float starting_x = rect.x + rect.w - (float)config->width.right;
        const float starting_y = rect.y + clampedRadii.topRight;
        const float length = rect.h - clampedRadii.topRight - clampedRadii.bottomRight;
        SDL_FRect line = { starting_x, starting_y, config->width.right, length };
        // SDL_RenderFillRect(rendererData->renderer, &line);
        GL_FillRoundedRect(rendererData, line, 0, zIndex, color);
      }
      if (config->width.top > 0) {
        const float starting_x = rect.x + clampedRadii.topLeft;
        const float length = rect.w - clampedRadii.topLeft - clampedRadii.topRight;
        SDL_FRect line = { starting_x, rect.y, length, config->width.top };
        // SDL_RenderFillRect(rendererData->renderer, &line);
        GL_FillRoundedRect(rendererData, line, 0, zIndex, color);
      }
      if (config->width.bottom > 0) {
        const float starting_x = rect.x + clampedRadii.bottomLeft;
        const float starting_y = rect.y + rect.h - (float)config->width.bottom;
        const float length = rect.w - clampedRadii.bottomLeft - clampedRadii.bottomRight;
        SDL_FRect line = { starting_x, starting_y, length, config->width.bottom };
        // SDL_SetRenderDrawColor(
        // rendererData->renderer, config->color.r, config->color.g, config->color.b, config->color.a);
        // SDL_RenderFillRect(rendererData->renderer, &line);
        GL_FillRoundedRect(rendererData, line, 0, zIndex, color);
      }
      // corners
      if (config->cornerRadius.topLeft > 0) {
        const float centerX = rect.x + clampedRadii.topLeft - 1;
        const float centerY = rect.y + clampedRadii.topLeft;
        // SDL_Clay_RenderArc(rendererData, (SDL_FPoint) { centerX, centerY }, clampedRadii.topLeft, 180.0f, 270.0f,
        //     config->width.top, config->color);
      }
      if (config->cornerRadius.topRight > 0) {
        const float centerX = rect.x + rect.w - clampedRadii.topRight - 1;
        const float centerY = rect.y + clampedRadii.topRight;
        // SDL_Clay_RenderArc(rendererData, (SDL_FPoint) { centerX, centerY }, clampedRadii.topRight, 270.0f, 360.0f,
        //     config->width.top, config->color);
      }
      if (config->cornerRadius.bottomLeft > 0) {
        const float centerX = rect.x + clampedRadii.bottomLeft - 1;
        const float centerY = rect.y + rect.h - clampedRadii.bottomLeft - 1;
        // SDL_Clay_RenderArc(rendererData, (SDL_FPoint) { centerX, centerY }, clampedRadii.bottomLeft, 90.0f, 180.0f,
        //     config->width.bottom, config->color);
      }
      if (config->cornerRadius.bottomRight > 0) {
        const float centerX
            = rect.x + rect.w - clampedRadii.bottomRight - 1; // TODO: why need to -1 in all calculations???
        const float centerY = rect.y + rect.h - clampedRadii.bottomRight - 1;
        // SDL_Clay_RenderArc(rendererData, (SDL_FPoint) { centerX, centerY }, clampedRadii.bottomRight, 0.0f, 90.0f,
        //     config->width.bottom, config->color);
      }

    } break;
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
      // Clay_BoundingBox boundingBox = rcmd->boundingBox;
      // currentClippingRectangle = (SDL_Rect) {
      //   .x = boundingBox.x,
      //   .y = boundingBox.y,
      //   .w = boundingBox.width,
      //   .h = boundingBox.height,
      // };
      // SDL_SetRenderClipRect(rendererData->renderer, &currentClippingRectangle);
    } break;
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
      // SDL_SetRenderClipRect(rendererData->renderer, NULL);
    } break;
    case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
      // SDL_Texture* texture = (SDL_Texture*)rcmd->renderData.image.imageData;
      // const SDL_FRect dest = { rect.x, rect.y, rect.w, rect.h };
      // SDL_RenderTexture(rendererData->renderer, texture, NULL, &dest);
    } break;
    default:
      SDL_Log("Unknown render command type: %d", rcmd->commandType);
    }
  }
}