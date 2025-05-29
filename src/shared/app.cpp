
#include "app.h"

Vec2i Page::getRenderSizePx(App* app)
{
  float pageWidthPx = 210 / document->zoomMmPerPx;
  float pageHeightPx = 297 / document->zoomMmPerPx;
  return Vec2i(pageWidthPx, pageHeightPx);
}

Vec2i Page::getTopLeftPx(App* app)
{
  auto pos = document->position;
  pos.y += pageNumId * getRenderSizePx(app).y * (1 + app->pageGapPercentOfHeight / 100.f);
  return Vec2i(pos.x, pos.y);
}

bool Page::overlapsWithViewport(App* app)
{
  auto& bb = app->mainViewportBB;
  auto topLeft = getTopLeftPx(app);
  auto bottomRight = topLeft + getRenderSizePx(app);
  if (bottomRight.x > 0 && bottomRight.y > 0 && topLeft.x < bb.width && topLeft.y < bb.height) {
    return true;
  }
  return false;
}