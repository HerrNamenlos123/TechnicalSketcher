
#include "../app.h"
#include "documentrenderer.cpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

void addDocument(Appstate* appstate)
{
  Document document;
  document.paperColor = Color(255, 255, 255, 255);
  appstate->documents.push_back(std::move(document));
}
void addPageToDocument(Appstate* appstate, Document& document)
{
  Page page;
  document.pages.push_back(std::move(page));
}

void handleZoomPan(Appstate* appstate)
{
  auto& document = appstate->documents[appstate->selectedDocument];
  int w, h;
  SDL_GetWindowSize(appstate->window, &w, &h);
  if (appstate->touchFingers.size() == 1) {
    auto finger = appstate->touchFingers[0];
    auto dx_px = finger.dx * w;
    auto dy_px = finger.dy * h;
    appstate->documents[appstate->selectedDocument].position.x += dx_px;
    appstate->documents[appstate->selectedDocument].position.y += dy_px;
  }
  else if (appstate->touchFingers.size() == 2) {
    Vec2 averagePosition = Vec2(0, 0);
    for (auto& finger : appstate->touchFingers) {
      auto x_px = finger.x * w - appstate->mainViewportBB.x;
      auto y_px = finger.y * h - appstate->mainViewportBB.y;
      averagePosition.x += x_px / appstate->touchFingers.size();
      averagePosition.y += y_px / appstate->touchFingers.size();
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
  }
  else if (appstate->touchFingers.size() == 0) {
    appstate->prevAveragePos = {};
    appstate->prevPinchDistance = {};
    return;
  }
}

void processFingerDownEvent(Appstate* appstate, SDL_TouchFingerEvent event)
{
  for (auto& finger : appstate->touchFingers) {
    if (finger.fingerID == event.fingerID) {
      finger = event;
    }
    else {
      finger.dx = 0;
      finger.dy = 0;
    }
  }
  appstate->touchFingers.push_back(event);
  appstate->prevAveragePos = {};
  appstate->prevPinchDistance = {};
  handleZoomPan(appstate);
}

void processFingerMotionEvent(Appstate* appstate, SDL_TouchFingerEvent event)
{
  for (auto& finger : appstate->touchFingers) {
    if (finger.fingerID == event.fingerID) {
      finger = event;
      handleZoomPan(appstate);
      return;
    }
  }
}

void processFingerUpEvent(Appstate* appstate, SDL_TouchFingerEvent event)
{
  appstate->touchFingers.erase(std::remove_if(appstate->touchFingers.begin(),
                                              appstate->touchFingers.end(),
                                              [&](SDL_TouchFingerEvent e) { return e.fingerID == event.fingerID; }),
                               appstate->touchFingers.end());
  appstate->prevAveragePos = {};
  appstate->prevPinchDistance = {};
  handleZoomPan(appstate);
}

void processFingerCancelledEvent(Appstate* appstate, SDL_TouchFingerEvent event)
{
  appstate->touchFingers.erase(std::remove_if(appstate->touchFingers.begin(),
                                              appstate->touchFingers.end(),
                                              [&](SDL_TouchFingerEvent e) { return e.fingerID == event.fingerID; }),
                               appstate->touchFingers.end());
  appstate->prevAveragePos = {};
  appstate->prevPinchDistance = {};
  handleZoomPan(appstate);
}