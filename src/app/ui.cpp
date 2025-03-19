
#include "../shared/app.h"
#include "components.cpp"

void ui(App* app)
{
  div(app, "w-full bg-white h-full col"s, [](App* app) {
    div(app, "bg-[#333] w-full h-[50px]"s, [](App* app) { });
    div(app, "w-full h-full"s, [](App* app) {
      div(app, "h-full bg-[#333] w-[200px] text-white"s, [](App* app) {
        text(app, ""s, "Hallo test"s);
      });
      div(app, "id-editor-viewport h-full w-full"s, [](App* app) {
        div(app, "h-full w-full"s, [](App* app) { }, app->mainDocumentRenderTexture);
      });
    });
  });
  Clay_ElementData viewportSize = Clay_GetElementData(CLAY_ID("editor-viewport"));
  app->mainViewportBB = viewportSize.boundingBox;
}