
#include "../shared/app.h"
#include "documentrenderer.cpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pen.h>
#include <SDL3/SDL_video.h>

void addDocument(App* appstate)
{
  Document document = (Document) {
    .pageWidthPercentOfWindow = 50,
    .pageScroll = 0,
    .position = Vec2(),
    .pages = {},
    .paperColor = Color(255, 255, 255, 255),
    .arena = Arena::create(),
  };
  appstate->documents.push(appstate->persistentApplicationArena, document);
}
void addPageToDocument(App* appstate, Document& document)
{
  Page page = (Page) {
    .shapes = {},
    .canvas = {},
  };
  document.pages.push(document.arena, page);
}

void handleZoomPan(App* appstate)
{
  auto& document = appstate->documents[appstate->selectedDocument];
  int w, h;
  SDL_GetWindowSize(appstate->window, &w, &h);
  if (appstate->touchFingers.length == 1) {
    auto finger = appstate->touchFingers[0];
    auto dx_px = finger.dx * w;
    auto dy_px = finger.dy * h;
    appstate->documents[appstate->selectedDocument].position.x += dx_px;
    appstate->documents[appstate->selectedDocument].position.y += dy_px;
  } else if (appstate->touchFingers.length == 2) {
    Vec2 averagePosition = Vec2(0, 0);
    for (auto& finger : appstate->touchFingers) {
      auto x_px = finger.x * w - appstate->mainViewportBB.x;
      auto y_px = finger.y * h - appstate->mainViewportBB.y;
      averagePosition.x += x_px / appstate->touchFingers.length;
      averagePosition.y += y_px / appstate->touchFingers.length;
    }

    float pinchDistance = Vec2(appstate->touchFingers[1].x * w - appstate->touchFingers[0].x * w,
        appstate->touchFingers[1].y * h - appstate->touchFingers[0].y * h)
                              .length();

    if (appstate->prevAveragePos) {
      document.position.x += averagePosition.x - appstate->prevAveragePos->x;
      document.position.y += averagePosition.y - appstate->prevAveragePos->y;
    }

    if (appstate->prevPinchDistance) {
      auto ratio = pinchDistance / *appstate->prevPinchDistance;
      document.pageWidthPercentOfWindow *= ratio;

      auto averageToOrigin = document.position - averagePosition;
      auto scaledAverageToOrigin = averageToOrigin * ratio;
      auto newOrigin = averagePosition + scaledAverageToOrigin;
      document.position = newOrigin;
    }

    appstate->prevAveragePos = averagePosition;
    appstate->prevPinchDistance = pinchDistance;
  } else if (appstate->touchFingers.length == 0) {
    appstate->prevAveragePos = {};
    appstate->prevPinchDistance = {};
    return;
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

void processFingerMotionEvent(App* appstate, SDL_TouchFingerEvent event)
{
  for (auto& finger : appstate->touchFingers) {
    if (finger.fingerID == event.fingerID) {
      finger = event;
      handleZoomPan(appstate);
      return;
    }
  }
}

void processFingerUpEvent(App* appstate, SDL_TouchFingerEvent event)
{
  // appstate->touchFingers.erase(std::remove_if(appstate->touchFingers.begin(),
  //                                  appstate->touchFingers.end(),
  //                                  [&](SDL_TouchFingerEvent e) { return e.fingerID == event.fingerID; }),
  //     appstate->touchFingers.end());
  appstate->prevAveragePos = {};
  appstate->prevPinchDistance = {};
  handleZoomPan(appstate);
}

void processFingerCancelledEvent(App* appstate, SDL_TouchFingerEvent event)
{
  // appstate->touchFingers.erase(std::remove_if(appstate->touchFingers.begin(),
  //                                  appstate->touchFingers.end(),
  //                                  [&](SDL_TouchFingerEvent e) { return e.fingerID == event.fingerID; }),
  //     appstate->touchFingers.end());
  appstate->prevAveragePos = {};
  appstate->prevPinchDistance = {};
  handleZoomPan(appstate);
}

void processPenAxisEvent(App* appstate, SDL_PenAxisEvent event)
{
  if (event.axis != SDL_PEN_AXIS_PRESSURE) {
    return;
  }
  appstate->currentPenPressure = event.value;
}

void processPenDownEvent(App* appstate, SDL_PenTouchEvent event)
{
  auto& document = appstate->documents[appstate->selectedDocument];
  int pageWidthPx = document.pageWidthPercentOfWindow * appstate->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;
  int pageXOffset = document.position.x;
  int pageYOffset = document.position.y;

  int pageIndex = 0;
  for (auto& page : document.pages) {
    Vec2 penPosition = Vec2(event.x - appstate->mainViewportBB.x, event.y - appstate->mainViewportBB.y);
    Vec2 pageTopLeftPx = Vec2(pageXOffset, pageYOffset);
    Vec2 pageBottomRightPx = Vec2(pageXOffset + pageWidthPx, pageYOffset + pageHeightPx);
    Vec2 penPosOnPagePx = penPosition - pageTopLeftPx;
    Vec2 penPosOnPage_mm = Vec2(penPosOnPagePx.x * 210 / pageWidthPx, penPosOnPagePx.y * 297 / pageHeightPx);

    if (penPosOnPage_mm.x >= 0 && penPosOnPage_mm.x <= 210 && penPosOnPage_mm.y >= 0 && penPosOnPage_mm.y <= 297) {
      appstate->currentlyDrawingOnPage = pageIndex;
      appstate->currentLine = LineShape();
      return;
    }
    pageYOffset += pageHeightPx + pageHeightPx * appstate->pageGapPercentOfHeight / 100;
    pageIndex++;
  }
  appstate->currentlyDrawingOnPage = -1;
}

void processPenUpEvent(App* appstate, SDL_PenTouchEvent event)
{
  appstate->currentlyDrawingOnPage = -1;
}

void processPenMotionEvent(App* appstate, SDL_PenMotionEvent event)
{
  auto& document = appstate->documents[appstate->selectedDocument];
  int pageWidthPx = document.pageWidthPercentOfWindow * appstate->mainViewportBB.width / 100.0;
  int pageHeightPx = pageWidthPx * 297 / 210;
  int pageXOffset = document.position.x;
  int pageYOffset = document.position.y;

  int pageIndex = 0;
  for (auto& page : document.pages) {
    Vec2 penPosition = Vec2(event.x - appstate->mainViewportBB.x, event.y - appstate->mainViewportBB.y);
    Vec2 pageTopLeftPx = Vec2(pageXOffset, pageYOffset);
    Vec2 pageBottomRightPx = Vec2(pageXOffset + pageWidthPx, pageYOffset + pageHeightPx);
    Vec2 penPosOnPagePx = penPosition - pageTopLeftPx;
    Vec2 penPosOnPage_mm = Vec2(penPosOnPagePx.x * 210 / pageWidthPx, penPosOnPagePx.y * 297 / pageHeightPx);

    if (penPosOnPage_mm.x >= 0 && penPosOnPage_mm.x <= 210 && penPosOnPage_mm.y >= 0 && penPosOnPage_mm.y <= 297) {
      InterpolationPoint point;
      point.pos = penPosOnPage_mm;
      point.thickness = appstate->currentPenPressure;
      appstate->currentLine.points.push(document.arena, point);
    }
    pageYOffset += pageHeightPx + pageHeightPx * appstate->pageGapPercentOfHeight / 100;
    pageIndex++;
  }
}

void processPenButtonDownEvent(App* appstate, SDL_PenButtonEvent event)
{
}

void processPenButtonUpEvent(App* appstate, SDL_PenButtonEvent event)
{
}