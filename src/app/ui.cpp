
#include "../app.h"
#include "components.cpp"

void ui(Appstate* appstate)
{
  div(appstate, "w-full bg-white h-full col", [](Appstate* appstate) {
    div(appstate, "bg-[#333] w-full h-[50px]", [](Appstate* appstate) { });
    div(appstate, "w-full h-full", [](Appstate* appstate) {
      div(appstate, "h-full bg-[#333] w-[200px] text-white", [](Appstate* appstate) {
        text(appstate, "", "Hallo test");
      });
      div(appstate, "id-editor-viewport h-full w-full", [](Appstate* appstate) {
        div(appstate, "h-full w-full", [](Appstate* appstate) { }, appstate->mainDocumentRenderSurface);
      });
    });
  });
  Clay_ElementData viewportSize = Clay_GetElementData(CLAY_ID("editor-viewport"));
  appstate->mainDocumentRenderSurfaceSize = Vec2(viewportSize.boundingBox.width, viewportSize.boundingBox.height);
}