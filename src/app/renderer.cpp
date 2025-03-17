
#include "../app.h"
#include "../clay.h"
#include <SDL3/SDL_init.h>

auto COLORS = Map<String, Clay_Color>();
auto FONT_SIZES = Map<String, int>();

List<String> split(String s, String delimiter)
{
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  String token;
  List<String> res;

  while ((pos_end = s.find(delimiter, pos_start)) != String::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

void RenderBegin() { }

void RenderEnd() { }

Tuple<String, String> parseClass(String classString)
{
  String modifier;
  String cmd;
  auto elements = split(classString, ":");
  if (elements.size() == 1) {
    cmd = elements[0];
  }
  else if (elements.size() == 2) {
    modifier = elements[0];
    cmd = elements[1];
  }
  return std::make_tuple(modifier, cmd);
}

void div(Appstate* appstate, String classString, void (*cb)(Appstate* appstate))
{
  List<String> classes = split(classString, " ");
  Clay_SizingAxis width;
  Clay_SizingAxis height;
  Clay_Padding padding;
  Color backgroundColor;

  size_t textColorPushed = 0;
  size_t textSizePushed = 0;

  for (const auto& classString : classes) {
    const auto [modifier, cmd] = parseClass(classString);

    if (cmd == "w-full") {
      width = CLAY_SIZING_GROW(0);
    }
    else if (cmd == "h-full") {
      height = CLAY_SIZING_GROW(0);
    }
    else if (cmd.starts_with("p-[")) {
      auto unit = cmd.substr(3, cmd.length() - 4);
      if (unit.ends_with("px")) {
        try {
          auto value = std::stol(unit.substr(0, unit.length() - 2));
          padding = CLAY_PADDING_ALL(value);
        }
        catch (...) {
        }
      }
    }
    else if (cmd.starts_with("bg-")) {
      auto value = cmd.substr(3, cmd.length() - 3);
      if (COLORS.contains(value)) {
        backgroundColor = COLORS.at(value);
      }
    }
    else if (cmd.starts_with("bg-")) {
      auto value = cmd.substr(3, cmd.length() - 3);
      if (COLORS.contains(value)) {
        backgroundColor = COLORS.at(value);
      }
    }
    else if (cmd.starts_with("text-")) {
      auto value = cmd.substr(3, cmd.length() - 3);
      if (COLORS.contains(value)) {
        appstate->uiCache.textColorStack.push_back(COLORS.at(value));
        textColorPushed++;
      }
      else if (FONT_SIZES.contains(value)) {
        appstate->uiCache.textSizeStack.push_back(FONT_SIZES.at(value));
        textSizePushed++;
      }
    }
  }

  Color textColor = COLORS["black"];
  if (appstate->uiCache.textColorStack.size() > 0) {
    textColor = appstate->uiCache.textColorStack.back();
  }

  int textsize = FONT_SIZES["base"];
  if (appstate->uiCache.textSizeStack.size() > 0) {
    textsize = appstate->uiCache.textSizeStack.back();
  }

  CLAY({ .layout = { .sizing = { .width = width, .height = height },
                     .padding = CLAY_PADDING_ALL(16),
                     .childGap = 16,
                     .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } },
         .backgroundColor = backgroundColor })
  {
    if (cb) {
      cb(appstate);
    }
  }

  for (int i = 0; i < textColorPushed; i++) {
    appstate->uiCache.textColorStack.pop_back();
  }
  for (int i = 0; i < textSizePushed; i++) {
    appstate->uiCache.textSizeStack.pop_back();
  }
}

void text(Appstate* appstate, String classString, String content)
{
  List<String> classes = split(classString, " ");
  Clay_SizingAxis width;
  Clay_SizingAxis height;
  Clay_Padding padding;
  Color backgroundColor;

  size_t textColorPushed = 0;
  size_t textSizePushed = 0;

  for (const auto& classString : classes) {
    const auto [modifier, cmd] = parseClass(classString);

    if (cmd == "w-full") {
      width = CLAY_SIZING_GROW(0);
    }
    else if (cmd == "h-full") {
      height = CLAY_SIZING_GROW(0);
    }
    else if (cmd.starts_with("p-[")) {
      auto unit = cmd.substr(3, cmd.length() - 4);
      if (unit.ends_with("px")) {
        try {
          auto value = std::stol(unit.substr(0, unit.length() - 2));
          padding = CLAY_PADDING_ALL(value);
        }
        catch (...) {
        }
      }
    }
    else if (cmd.starts_with("bg-")) {
      auto value = cmd.substr(3, cmd.length() - 3);
      if (COLORS.contains(value)) {
        backgroundColor = COLORS.at(value);
      }
    }
    else if (cmd.starts_with("bg-")) {
      auto value = cmd.substr(3, cmd.length() - 3);
      if (COLORS.contains(value)) {
        backgroundColor = COLORS.at(value);
      }
    }
    else if (cmd.starts_with("text-")) {
      auto value = cmd.substr(5, cmd.length() - 5);
      if (COLORS.contains(value)) {
        appstate->uiCache.textColorStack.push_back(COLORS.at(value));
        textColorPushed++;
      }
      else if (FONT_SIZES.contains(value)) {
        appstate->uiCache.textSizeStack.push_back(FONT_SIZES.at(value));
        textSizePushed++;
      }
    }
  }

  Color textColor = COLORS["black"];
  if (appstate->uiCache.textColorStack.size() > 0) {
    textColor = appstate->uiCache.textColorStack.back();
  }

  int textsize = FONT_SIZES["base"];
  if (appstate->uiCache.textSizeStack.size() > 0) {
    textsize = appstate->uiCache.textSizeStack.back();
  }

  appstate->uiCache.stringCache.push_back(content);
  CLAY_TEXT(((Clay_String) {
                .length = appstate->uiCache.stringCache.back().length(),
                .chars = appstate->uiCache.stringCache.back().c_str(),
            }),
            CLAY_TEXT_CONFIG({
                .textColor = textColor,
                .fontSize = textsize,
            }));

  for (int i = 0; i < textColorPushed; i++) {
    appstate->uiCache.textColorStack.pop_back();
  }
  for (int i = 0; i < textSizePushed; i++) {
    appstate->uiCache.textSizeStack.pop_back();
  }
}

void ui(Appstate* appstate)
{
  div(appstate, "w-full bg-blue", [](Appstate* appstate) { text(appstate, "text-green text-base", "Hallo test"); });
  CLAY({ .id = CLAY_ID("SideBar"),
         .layout = { .sizing = { .width = CLAY_SIZING_FIXED(900), .height = CLAY_SIZING_FIXED(500) },
                     .padding = CLAY_PADDING_ALL(16),
                     .childGap = 16 ,
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
         .backgroundColor = (Clay_Color) { 255, 128, 128, 255 } })
  {
    CLAY({ .id = CLAY_ID("ProfilePictureOuter"),
           .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) },
                       .padding = CLAY_PADDING_ALL(16),
                       .childGap = 16,
                       .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } },
           .backgroundColor = (Clay_Color) { 255, 128, 128, 255 } })
    {
      CLAY_TEXT(CLAY_STRING("Clay - UI Library"),
                CLAY_TEXT_CONFIG({
                    .textColor = { 255, 255, 255, 255 },
                    .fontSize = 24,
                }));
    }
  }
}

static inline Clay_Dimensions SDL_MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, void* _appstate)
{
  Appstate* appstate = (Appstate*)_appstate;
  auto& fonts = appstate->rendererData.fonts;
  TTF_Font* font = std::get<TTF_Font*>(fonts[0]);
  for (auto [_font, size] : fonts) {
    if (size == config->fontSize) {
      font = _font;
    }
  }
  int width, height;

  if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s", SDL_GetError());
  }

  return (Clay_Dimensions) { (float)width, (float)height };
}

void HandleClayErrors(Clay_ErrorData errorData) { printf("%s", errorData.errorText.chars); }

extern "C" void InitClay(Appstate* appstate)
{
  uint64_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = (Clay_Arena) { .capacity = totalMemorySize, .memory = (char*)SDL_malloc(totalMemorySize) };

  int width, height;
  SDL_GetWindowSize(appstate->window, &width, &height);

  Clay_Initialize(
      clayMemory, (Clay_Dimensions) { (float)width, (float)height }, (Clay_ErrorHandler) { HandleClayErrors });
  Clay_SetMeasureTextFunction(SDL_MeasureText, appstate);

  COLORS["white"] = (Clay_Color) { 255, 255, 255, 255 };
  COLORS["black"] = (Clay_Color) { 0, 0, 0, 255 };
  COLORS["red"] = (Clay_Color) { 255, 0, 0, 255 };
  COLORS["green"] = (Clay_Color) { 0, 255, 0, 255 };
  COLORS["blue"] = (Clay_Color) { 0, 0, 255, 255 };
  FONT_SIZES["xs"] = 12;
  FONT_SIZES["sm"] = 14;
  FONT_SIZES["base"] = 16;
  FONT_SIZES["lg"] = 18;
  FONT_SIZES["xl"] = 20;
  FONT_SIZES["2xl"] = 24;
}

extern "C" SDL_AppResult EventHandler(Appstate* appstate, SDL_Event* event)
{
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
    break;

  case SDL_EVENT_WINDOW_RESIZED:
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)event->window.data1, (float)event->window.data2 });
    break;

  case SDL_EVENT_MOUSE_MOTION:
    Clay_SetPointerState((Clay_Vector2) { event->motion.x, event->motion.y }, event->motion.state & SDL_BUTTON_LMASK);
    break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    Clay_SetPointerState((Clay_Vector2) { event->button.x, event->button.y }, event->button.button == SDL_BUTTON_LEFT);
    break;

  case SDL_EVENT_MOUSE_WHEEL:
    Clay_UpdateScrollContainers(true, (Clay_Vector2) { event->wheel.x, event->wheel.y }, 0.01f);
    break;

  default:
    break;
  }
  return SDL_APP_CONTINUE;
}

extern "C" Clay_RenderCommandArray DrawUI(Appstate* appstate)
{
  Clay_BeginLayout();

  CLAY({
      .layout = { 
                  .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
        .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },})
  {
    ui(appstate);
  }

  //   ui();
  Clay_RenderCommandArray renderCommands = Clay_EndLayout();

  appstate->uiCache.stringCache.clear();
  appstate->uiCache.textColorStack.clear();
  appstate->uiCache.textSizeStack.clear();

  return renderCommands;
}

// void ApplicationRenderer::BeginFrame() { Battery::Renderer2D::BeginScene(GetInstance().scene.get()); }

// void ApplicationRenderer::EndFrame() { Battery::Renderer2D::EndScene(); }

// void ApplicationRenderer::DrawLineWorkspace(const glm::vec2& point1,
//                                             const glm::vec2& point2,
//                                             float thickness,
//                                             const glm::vec4& color,
//                                             float falloff)
// {
//   glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1);
//   glm::vec2 p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
//   float thick = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//   Battery::Renderer2D::DrawLine(p1, p2, std::max(thick, 0.5f), color, falloff);
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

//   glm::vec2 p1 = Battery::MathUtils::MapVector(point1, min, max, { 0, 0 }, { width, height });
//   glm::vec2 p2 = Battery::MathUtils::MapVector(point2, min, max, { 0, 0 }, { width, height });
//   float thick = Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

//   Battery::Renderer2D::DrawLine(p1, p2, thick, color, EXPORT_FALLOFF);
// }

// void ApplicationRenderer::DrawCircleWorkspace(const glm::vec2& center,
//                                               float radius,
//                                               float thickness,
//                                               const glm::vec4& color,
//                                               float falloff)
// {
//   glm::vec2 c = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center);
//   float r = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius);
//   float t = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//   Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 }, falloff);
// }

// void ApplicationRenderer::DrawCircleScreenspace(const glm::vec2& center,
//                                                 float radius,
//                                                 float thickness,
//                                                 const glm::vec4& color,
//                                                 float falloff)
// {
//   Battery::Renderer2D::DrawCircle(center, radius, thickness, color, { 0, 0, 0, 0 }, falloff);
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

//   glm::vec2 c = Battery::MathUtils::MapVector(center, min, max, { 0, 0 }, { width, height });
//   float r = Battery::MathUtils::MapFloat(radius, 0, max.x - min.x, 0, width);
//   float t = Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

//   Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 }, EXPORT_FALLOFF);
// }

// void ApplicationRenderer::DrawArcWorkspace(const glm::vec2& center,
//                                            float radius,
//                                            float startAngle,
//                                            float endAngle,
//                                            float thickness,
//                                            const glm::vec4& color,
//                                            float falloff)
// {
//   glm::vec2 c = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center);
//   float r = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius);
//   float t = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//   Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color, falloff);
// }

// void ApplicationRenderer::DrawArcScreenspace(const glm::vec2& center,
//                                              float radius,
//                                              float startAngle,
//                                              float endAngle,
//                                              float thickness,
//                                              const glm::vec4& color,
//                                              float falloff)
// {
//   Battery::Renderer2D::DrawArc(center, radius, startAngle, endAngle, thickness, color, falloff);
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

//   glm::vec2 c = Battery::MathUtils::MapVector(center, min, max, { 0, 0 }, { width, height });
//   float r = Battery::MathUtils::MapFloat(radius, 0, max.x - min.x, 0, width);
//   float t = Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

//   Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color, EXPORT_FALLOFF);
// }

// void ApplicationRenderer::DrawRectangleWorkspace(const glm::vec2& point1,
//                                                  const glm::vec2& point2,
//                                                  float outlineThickness,
//                                                  const glm::vec4& outlineColor,
//                                                  const glm::vec4& fillColor,
//                                                  float falloff)
// {
//   glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1);
//   glm::vec2 p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
//   Battery::Renderer2D::DrawRectangle(p1, p2, outlineThickness, outlineColor, fillColor, falloff);
// }

// void ApplicationRenderer::DrawRectangleScreenspace(const glm::vec2& point1,
//                                                    const glm::vec2& point2,
//                                                    float outlineThickness,
//                                                    const glm::vec4& outlineColor,
//                                                    const glm::vec4& fillColor,
//                                                    float falloff)
// {
//   Battery::Renderer2D::DrawRectangle(point1, point2, outlineThickness, outlineColor, fillColor, falloff);
// }

// void ApplicationRenderer::DrawSelectionBoxInfillRectangle(const glm::vec2& point1, const glm::vec2& point2)
// {
//   ApplicationRenderer::DrawRectangleWorkspace(
//       point1, point2, 0, { 0, 0, 0, 0 }, GetInstance().selectionBoxFillColor, 0);
// }

// void ApplicationRenderer::DrawSelectionBoxOutlineRectangle(const glm::vec2& point1, const glm::vec2& point2)
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
//   glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(position);
//   ApplicationRenderer::DrawRectangleScreenspace(
//       p1 - glm::vec2(GetInstance().previewPointSize / 2, GetInstance().previewPointSize / 2),
//       p1 + glm::vec2(GetInstance().previewPointSize / 2, GetInstance().previewPointSize / 2),
//       1,
//       { 0, 0, 0, 255 },
//       { 255, 255, 255, 255 },
//       0);
// }

// void ApplicationRenderer::DrawGrid(bool infinite)
// {
//   using namespace Battery;
//   auto nav = Navigator::GetInstance();

//   float thickness = GetInstance().gridLineWidth;
//   float alpha
//       = std::min(Navigator::GetInstance()->scale * GetInstance().gridAlphaFactor + GetInstance().gridAlphaOffset,
//                  GetInstance().gridAlphaMax);
//   glm::vec4 color
//       = glm::vec4(GetInstance().gridLineColor, GetInstance().gridLineColor, GetInstance().gridLineColor, alpha);

//   int w = GetMainWindow().GetWidth();
//   int h = GetMainWindow().GetHeight();

//   float right = w;
//   float left = 0;
//   float top = h;
//   float bottom = 0;

//   if (!infinite) { // Draw an A4-sheet
//     glm::vec2 sheetSize = { 210, 297 };
//     right = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(sheetSize / 2.f).x;
//     left = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize / 2.f).x;
//     top = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(sheetSize / 2.f).y;
//     bottom = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize / 2.f).y;

//     for (float x = nav->panOffset.x + w / 2; x < right; x += nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//     }
//     for (float x = nav->panOffset.x + w / 2 - nav->scale * nav->snapSize; x > left; x -= nav->scale * nav->snapSize)
//     {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//     }
//     for (float y = nav->panOffset.y + h / 2; y < top; y += nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//     }
//     for (float y = nav->panOffset.y + h / 2 - nav->scale * nav->snapSize; y > bottom; y -= nav->scale *
//     nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//     }
//   }
//   else {

//     for (float x = nav->panOffset.x + w / 2; x < right; x += nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//     }
//     for (float x = nav->panOffset.x + w / 2; x > left; x -= nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//     }
//     for (float y = nav->panOffset.y + h / 2; y < top; y += nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//     }
//     for (float y = nav->panOffset.y + h / 2; y > bottom; y -= nav->scale * nav->snapSize) {
//       Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//     }
//   }

//   if (!infinite) { // Draw sheet outline
//     Renderer2D::DrawPrimitiveLine({ left, bottom }, { right, bottom }, thickness * 2, color);
//     Renderer2D::DrawPrimitiveLine({ right, bottom }, { right, top }, thickness * 2, color);
//     Renderer2D::DrawPrimitiveLine({ right, top }, { left, top }, thickness * 2, color);
//     Renderer2D::DrawPrimitiveLine({ left, top }, { left, bottom }, thickness * 2, color);
//   }
// }
