
#include "../shared/app.h"
#include "../shared/gl.hpp"
#include "math.h"
#include "profiling.cpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pen.h>
#include <SDL3/SDL_video.h>

#include "cJSON.h"

const auto RAMER_DOUGLAS_PEUCKER_SMOOTHING = 0.2;
const auto DOCUMENT_START_POSITION = Vec2(300, 100);
const auto DOCUMENT_DEFAULT_ZOOM_MM_PER_PX = 0.2;

void addDocument(App* app)
{
  Document document = Document {
    .zoomMmPerPx = DOCUMENT_DEFAULT_ZOOM_MM_PER_PX,
    .pageScroll = 0,
    .position = DOCUMENT_START_POSITION,
    .pages = {},
    .paperColor = Color(255, 255, 255, 255),
    .arena = Arena::create(),
  };
  app->documents.push(app->persistentApplicationArena, document);
}

void unloadDocument(App* app, Document& document)
{
  for (auto& page : document.pages) {
    page.tempRenderTexture.free();
    page.persistentFBO.free();
  }
  document.arena.free();
}

void addEmptyPageToDocument(App* app, Document& document)
{
  Page page = Page {
    .document = &document,
    .pageNumId = document.pages.length,
    .shapes = {},
    .persistentFBO = gl::Framebuffer::create(),
    .previewFBO = gl::Framebuffer::create(),
  };

  document.pages.push(document.arena, page);
}

void saveDocumentToFile(App* app, Document& document, String filepath)
{
  static Arena* arena;
  Arena _arena = Arena::create();
  arena = &_arena;
  cJSON_Hooks hooks = {
    .malloc_fn = [](size_t size) -> void* { return arena->allocate<char>(size); },
    .free_fn = [](void* ptr) {},
  };
  cJSON_InitHooks(&hooks);

  auto file = cJSON_CreateObject();
  cJSON_AddStringToObject(file, "filetype", "technicalsketcher");
  cJSON_AddNumberToObject(file, "fileversion", 1);
  cJSON_AddStringToObject(file, "papercolor", document.paperColor.toHex(*arena).c_str(*arena));

  auto pages = cJSON_AddArrayToObject(file, "pages");

  for (auto& page : document.pages) {
    auto pageJson = cJSON_CreateObject();
    auto shapesArray = cJSON_AddArrayToObject(pageJson, "shapes");
    for (auto& shape : page.shapes) {
      auto shapeJson = cJSON_CreateObject();
      cJSON_AddStringToObject(shapeJson, "color", shape.color.toHex(*arena).c_str(*arena));
      auto pointsArray = cJSON_AddArrayToObject(shapeJson, "points");
      for (auto& point : shape.points) {
        auto pointJson = cJSON_CreateObject();
        cJSON_AddNumberToObject(pointJson, "x", point.pos_mm_scaled.x / app->perfectFreehandAccuracyScaling);
        cJSON_AddNumberToObject(pointJson, "y", point.pos_mm_scaled.y / app->perfectFreehandAccuracyScaling);
        cJSON_AddNumberToObject(pointJson, "pressure", point.pressure);
        cJSON_AddItemToArray(pointsArray, pointJson);
      }
      cJSON_AddItemToArray(shapesArray, shapeJson);
    }
    cJSON_AddItemToArray(pages, pageJson);
  }

  const char* jsonContent = cJSON_PrintUnformatted(file);

  FILE* f = fopen(filepath.c_str(*arena), "w");
  if (!f) {
    ts::panic("File failed to write");
  }
  fprintf(f, "%s", jsonContent);
  fclose(f);

  arena->free();
}

void openDocumentFromFile(App* app, String filepath)
{
  static Arena* arena;
  Arena _arena = Arena::create();
  arena = &_arena;
  cJSON_Hooks hooks = {
    .malloc_fn = [](size_t size) -> void* { return arena->allocate<char>(size); },
    .free_fn = [](void* ptr) {},
  };
  cJSON_InitHooks(&hooks);

  for (auto& doc : app->documents) {
    unloadDocument(app, doc);
  }
  app->documents.clear();

  auto file = ts::fs::read(*arena, filepath);
  if (!file) {
    ts::panic("File failed to read");
  }

  auto json = cJSON_Parse(file->c_str(*arena));

  auto filetype = cJSON_GetStringValue(cJSON_GetObjectItem(json, "filetype"));
  if (String::view(filetype) != "technicalsketcher") {
    ts::panic("Unexpected file type");
  }
  auto fileversion = cJSON_GetNumberValue(cJSON_GetObjectItem(json, "fileversion"));
  if (fileversion != 1.0) {
    ts::panic("Unexpected file version");
  }

  Document _document = Document {
    .zoomMmPerPx = DOCUMENT_DEFAULT_ZOOM_MM_PER_PX,
    .pageScroll = 0,
    .position = DOCUMENT_START_POSITION,
    .pages = {},
    .paperColor = Color(cJSON_GetStringValue(cJSON_GetObjectItem(json, "papercolor"))),
    .arena = Arena::create(),
  };
  app->documents.push(app->persistentApplicationArena, _document);
  auto& document = app->documents.back();

  auto pagesArray = cJSON_GetObjectItem(json, "pages");
  auto numPages = cJSON_GetArraySize(pagesArray);

  for (int i = 0; i < numPages; i++) {
    auto pageJson = cJSON_GetArrayItem(pagesArray, i);

    document.pages.push(document.arena,
        Page {
            .document = &document,
            .pageNumId = i,
            .shapes = {},
            .persistentFBO = gl::Framebuffer::create(),
            .previewFBO = gl::Framebuffer::create(),
        });
    Page& page = document.pages.back();

    auto shapesArray = cJSON_GetObjectItem(pageJson, "shapes");
    auto numShapes = cJSON_GetArraySize(shapesArray);

    for (int i = 0; i < numShapes; i++) {
      auto shapeJson = cJSON_GetArrayItem(shapesArray, i);
      LineShape shape = LineShape {
        .points = {},
        .color = cJSON_GetStringValue(cJSON_GetObjectItem(shapeJson, "color")),
        .prerendered = false,
      };

      auto pointsArray = cJSON_GetObjectItem(shapeJson, "points");
      auto numPoints = cJSON_GetArraySize(pointsArray);

      for (int i = 0; i < numPoints; i++) {
        auto pointJson = cJSON_GetArrayItem(pointsArray, i);
        shape.points.push(document.arena,
            {
                .pos_mm_scaled
                = Vec2(cJSON_GetNumberValue(cJSON_GetObjectItem(pointJson, "x")) * app->perfectFreehandAccuracyScaling,
                    cJSON_GetNumberValue(cJSON_GetObjectItem(pointJson, "y")) * app->perfectFreehandAccuracyScaling),
                .pressure = cJSON_GetNumberValue(cJSON_GetObjectItem(pointJson, "pressure")),
            });
      }

      page.shapes.push(document.arena, shape);
    }
  }

  arena->free();
}

void zoomInAtPoint(App* app, double amount, Vec2 point)
{
  auto& document = app->documents[app->selectedDocument];
  document.zoomMmPerPx /= amount;

  auto averageToOrigin = document.position - point;
  auto scaledAverageToOrigin = averageToOrigin * amount;
  auto newOrigin = point + scaledAverageToOrigin;
  document.position = newOrigin;
}

void panDocument(App* app, Vec2 delta)
{
  app->documents[app->selectedDocument].position += delta;
}

void handleZoomPan(App* app)
{
  auto& document = app->documents[app->selectedDocument];
  int w, h;
  SDL_GetWindowSize(app->window, &w, &h);
  if (app->touchFingers.length == 1) {
    auto finger = app->touchFingers[0];
    panDocument(app, { finger.dx * w, finger.dy * h });
  } else if (app->touchFingers.length == 2) {
    Vec2 averagePosition = Vec2(0, 0);
    for (auto& finger : app->touchFingers) {
      auto x_px = finger.x * w - app->mainViewportBB.x;
      auto y_px = finger.y * h - app->mainViewportBB.y;
      averagePosition.x += x_px / app->touchFingers.length;
      averagePosition.y += y_px / app->touchFingers.length;
    }

    float pinchDistance = Vec2(app->touchFingers[1].x * w - app->touchFingers[0].x * w,
        app->touchFingers[1].y * h - app->touchFingers[0].y * h)
                              .length();

    if (app->prevAveragePos) {
      panDocument(app, averagePosition - *app->prevAveragePos);
    }

    if (app->prevPinchDistance) {
      auto ratio = pinchDistance / *app->prevPinchDistance;
      zoomInAtPoint(app, ratio, averagePosition);
    }

    app->prevAveragePos = averagePosition;
    app->prevPinchDistance = pinchDistance;
  } else if (app->touchFingers.length == 0) {
    app->prevAveragePos = {};
    app->prevPinchDistance = {};
    return;
  }
}

void processMouseWheelEvent(App* app, SDL_MouseWheelEvent event)
{
  float scrollSpeed = 100;
  float zoomSpeed = 0.1;
  if (app->inputs.ctrl) {
    auto& document = app->documents[app->selectedDocument];
    auto amount = 1 + event.y * zoomSpeed;
    auto mouse = Vec2(event.mouse_x, event.mouse_y);
    zoomInAtPoint(app, amount, mouse - Vec2(app->mainViewportBB.x, app->mainViewportBB.y));
  } else {
    app->documents[app->selectedDocument].position.y += event.y * scrollSpeed;
  }
}

void processFingerDownEvent(App* app, SDL_TouchFingerEvent event)
{
  for (auto& finger : app->touchFingers) {
    if (finger.fingerID == event.fingerID) {
      finger = event;
    } else {
      finger.dx = 0;
      finger.dy = 0;
    }
  }
  app->touchFingers.push(app->persistentApplicationArena, event);
  app->prevAveragePos = {};
  app->prevPinchDistance = {};
  handleZoomPan(app);
}

void processFingerMotionEvent(App* app, SDL_TouchFingerEvent event)
{
  for (auto& finger : app->touchFingers) {
    if (finger.fingerID == event.fingerID) {
      finger = event;
      handleZoomPan(app);
      return;
    }
  }
}

void processFingerUpEvent(App* app, SDL_TouchFingerEvent event)
{
  app->touchFingers.remove_if([&](auto& elem) -> bool { return elem.fingerID == event.fingerID; });
  app->prevAveragePos = {};
  app->prevPinchDistance = {};
  handleZoomPan(app);
}

void processFingerCancelledEvent(App* app, SDL_TouchFingerEvent event)
{
  app->touchFingers.remove_if([&](auto& elem) -> bool { return elem.fingerID == event.fingerID; });
  app->prevAveragePos = {};
  app->prevPinchDistance = {};
  handleZoomPan(app);
}

void processPenAxisEvent(App* app, SDL_PenAxisEvent event)
{
  if (event.axis != SDL_PEN_AXIS_PRESSURE) {
    return;
  }
  app->currentPenPressure = event.value;
}

void processPenDownEvent(App* app, SDL_PenTouchEvent event)
{
  auto& document = app->documents[app->selectedDocument];

  for (auto& page : document.pages) {
    Vec2 penPosition = Vec2(event.x - app->mainViewportBB.x, event.y - app->mainViewportBB.y);
    Vec2i pageSizeI = page.getRenderSizePx(app);
    Vec2 pageSize = { pageSizeI.x, pageSizeI.y };
    Vec2i topLeftI = page.getTopLeftPx(app);
    Vec2 topLeft = { topLeftI.x, topLeftI.y };
    auto bottomRight = topLeft + pageSize;
    Vec2 penPosOnPagePx = penPosition - topLeft;
    Vec2 penPosOnPage_mm = Vec2(penPosOnPagePx.x * 210 / pageSize.x, penPosOnPagePx.y * 297 / pageSize.y);

    if (penPosOnPage_mm.x >= 0 && penPosOnPage_mm.x <= 210 && penPosOnPage_mm.y >= 0 && penPosOnPage_mm.y <= 297) {
      app->currentlyDrawingOnPage = page.pageNumId;
      document.currentLine = {};
      document.currentLine.color = Color("#FF0000");
      return;
    }
  }
  app->currentlyDrawingOnPage = -1;
}

void processPenUpEvent(App* app, SDL_PenTouchEvent event)
{
  auto& document = app->documents[app->selectedDocument];
  if (app->currentlyDrawingOnPage == -1) {
    return;
  }

  auto& page = document.pages[app->currentlyDrawingOnPage];
  page.shapes.push(document.arena, document.currentLine);
  document.currentLine = {};
  app->currentlyDrawingOnPage = -1;

  // List<InterpolationPoint> result1;
  // result1.push(document.arena, document.currentLine.points[0]);
  // for (size_t i = 1; i < document.currentLine.points.length - 2; i++) {
  //   auto& prev = document.currentLine.points[i - 1];
  //   auto& point = document.currentLine.points[i];
  //   auto& next = document.currentLine.points[i + 1];
  //   point.pos = (prev.pos + next.pos) / 2.f;
  //   result1.push(document.arena, document.currentLine.points[i]);
  // }
  // result1.push(document.arena, document.currentLine.points.back());

  // List<Vec2> result;
  // rdp(document.arena, document.currentLine.points, RAMER_DOUGLAS_PEUCKER_SMOOTHING, result);
  // document.currentLine.points = result;
}

void processMouseMotionEvent(App* app, SDL_MouseMotionEvent event)
{
  auto& document = app->documents[app->selectedDocument];
  if (app->inputs.mousewheel) {
    panDocument(app, { event.xrel, event.yrel });
  }
}

void processPenMotionEvent(App* app, SDL_PenMotionEvent event)
{
  auto& document = app->documents[app->selectedDocument];

  if (app->currentlyDrawingOnPage == -1) {
    return;
  }

  if (event.pen_state & SDL_PEN_INPUT_DOWN) {
    for (auto& page : document.pages) {
      auto pageSize = page.getRenderSizePx(app);

      if (app->currentlyDrawingOnPage != page.pageNumId) {
        continue;
      }

      Vec2i topLeftI = page.getTopLeftPx(app);
      Vec2 topLeft = { topLeftI.x, topLeftI.y };
      Vec2 penPosition = Vec2(event.x - app->mainViewportBB.x, event.y - app->mainViewportBB.y);
      Vec2 penPosOnPagePx = penPosition - topLeft;
      Vec2 penPosOnPage_mm = Vec2(penPosOnPagePx.x * 210 / pageSize.x, penPosOnPagePx.y * 297 / pageSize.y);

      SamplePoint point;
      point.pos_mm_scaled = penPosOnPage_mm * app->perfectFreehandAccuracyScaling;
      point.pressure = app->currentPenPressure * app->penPressureScaling;
      document.currentLine.points.push(document.arena, point);
    }
  }
}

void processPenButtonDownEvent(App* appstate, SDL_PenButtonEvent event)
{
}

void processPenButtonUpEvent(App* appstate, SDL_PenButtonEvent event)
{
}