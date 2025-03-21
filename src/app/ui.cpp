
#include "../shared/app.h"
#include "components.cpp"

void ui(App* app)
{
  div(app, "w-full bg-white h-full col"_s, [](App* app) {
    div(app, "bg-[#333] w-full h-[50px]"_s, [](App* app) {});
    div(app, "w-full h-full"_s, [](App* app) {
      div(app, "h-full bg-[#333] w-[200px] text-white"_s, [](App* app) {
        text(app, ""_s, "Hallo test"_s);
      });
      div(app, "id-editor-viewport h-full w-full"_s, [](App* app) {
        div(
            app, "h-full w-full"_s, [](App* app) {}, app->mainDocumentRenderTexture);
      });
    });
  });
  Clay_ElementData viewportSize = Clay_GetElementData(CLAY_ID("editor-viewport"));
  app->mainViewportBB = viewportSize.boundingBox;
}