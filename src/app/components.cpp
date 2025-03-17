
#include "../app.h"
#include "uiCache.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_system.h>

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

Clay_String ClayString(Appstate* appstate, String str)
{
  appstate->uiCache->stringCache.push_back(str);
  return (Clay_String) {
    .length = appstate->uiCache->stringCache.back().length(),
    .chars = appstate->uiCache->stringCache.back().c_str(),
  };
}

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

uint8_t hexToDigit(char letter)
{
  if (letter >= '0' && letter <= '9') {
    return letter - '0';
  }
  else if (letter >= 'a' && letter <= 'f') {
    return letter - 'a' + 10;
  }
  else if (letter >= 'A' && letter <= 'F') {
    return letter - 'A' + 10;
  }
  else {
    return 0;
  }
}

Color parseHexcolor(String hex)
{
  if (hex.length() == 4) {
    uint8_t r = hexToDigit(hex[1]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[2]) + 16 * hexToDigit(hex[2]);
    uint8_t b = hexToDigit(hex[3]) + 16 * hexToDigit(hex[3]);
    uint8_t a = 255;
    return Color(r, g, b, a);
  }
  else if (hex.length() == 5) {
    uint8_t r = hexToDigit(hex[1]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[2]) + 16 * hexToDigit(hex[2]);
    uint8_t b = hexToDigit(hex[3]) + 16 * hexToDigit(hex[3]);
    uint8_t a = hexToDigit(hex[4]) + 16 * hexToDigit(hex[4]);
    return Color(r, g, b, a);
  }
  else if (hex.length() == 7) {
    uint8_t r = hexToDigit(hex[2]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[4]) + 16 * hexToDigit(hex[3]);
    uint8_t b = hexToDigit(hex[6]) + 16 * hexToDigit(hex[5]);
    uint8_t a = 255;
    return Color(r, g, b, a);
  }
  else if (hex.length() == 9) {
    uint8_t r = hexToDigit(hex[2]) + 16 * hexToDigit(hex[1]);
    uint8_t g = hexToDigit(hex[4]) + 16 * hexToDigit(hex[3]);
    uint8_t b = hexToDigit(hex[6]) + 16 * hexToDigit(hex[5]);
    uint8_t a = hexToDigit(hex[8]) + 16 * hexToDigit(hex[7]);
    return Color(r, g, b, a);
  }
  else {
    return Color(0, 0, 0, 0);
  }
}

Optional<Color> parseBgClass(String cls)
{
  if (cls.starts_with("bg-[")) {
    return parseHexcolor(cls.substr(4, cls.length() - 5));
  }
  else if (cls.starts_with("bg-")) {
    auto value = cls.substr(3, cls.length() - 3);
    if (COLORS.contains(value)) {
      return COLORS.at(value);
    }
  }
  return {};
}

void div(Appstate* appstate, String classString, void (*cb)(Appstate* appstate), Optional<SDL_Texture*> image = {})
{
  List<String> classes = split(classString, " ");
  Clay_SizingAxis width = {};
  Clay_SizingAxis height = {};
  Clay_Padding padding = {};
  Clay_LayoutDirection layoutDirection = CLAY_LEFT_TO_RIGHT;
  Clay_ChildAlignment childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER };
  Clay_ElementId id = {};
  Color backgroundColor = {};

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
    else if (cmd.starts_with("w-[")) {
      auto unit = cmd.substr(3, cmd.length() - 4);
      if (unit.ends_with("px")) {
        try {
          auto value = std::stol(unit.substr(0, unit.length() - 2));
          width = CLAY_SIZING_FIXED(value);
        }
        catch (...) {
        }
      }
    }
    else if (cmd.starts_with("h-[")) {
      auto unit = cmd.substr(3, cmd.length() - 4);
      if (unit.ends_with("px")) {
        try {
          auto value = std::stol(unit.substr(0, unit.length() - 2));
          height = CLAY_SIZING_FIXED(value);
        }
        catch (...) {
        }
      }
    }
    else if (cmd.starts_with("id-")) {
      auto value = cmd.substr(3, cmd.length() - 3);
      id = CLAY_SIDI(ClayString(appstate, value), 0);
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
    else if (cmd == "col") {
      layoutDirection = CLAY_TOP_TO_BOTTOM;
      childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_TOP };
    }
    else if (auto col = parseBgClass(cmd)) {
      backgroundColor = *col;
    }
    else if (cmd.starts_with("text-")) {
      auto value = cmd.substr(5, cmd.length() - 5);
      if (COLORS.contains(value)) {
        appstate->uiCache->textColorStack.push_back(COLORS.at(value));
        textColorPushed++;
      }
      else if (FONT_SIZES.contains(value)) {
        appstate->uiCache->textSizeStack.push_back(FONT_SIZES.at(value));
        textSizePushed++;
      }
    }
  }

  Color textColor = COLORS["black"];
  if (appstate->uiCache->textColorStack.size() > 0) {
    textColor = appstate->uiCache->textColorStack.back();
  }

  int textsize = FONT_SIZES["base"];
  if (appstate->uiCache->textSizeStack.size() > 0) {
    textsize = appstate->uiCache->textSizeStack.back();
  }

  Clay_ImageElementConfig imageConfig = {};
  if (image && *image) {
    auto texture = appstate->mainDocumentRenderTexture;
    imageConfig = { .imageData = *image, .sourceDimensions = { texture->w, texture->h } };
  }

  CLAY({
    .id = id,
    .layout = { .sizing = { .width = width, .height = height },
                     .padding = CLAY_PADDING_ALL(0),
                     .childGap = 0,
                     .childAlignment = childAlignment,
                     .layoutDirection = layoutDirection,
                     },
         .backgroundColor = backgroundColor,
    .image = imageConfig,
  })
  {
    if (cb) {
      cb(appstate);
    }
  }

  for (int i = 0; i < textColorPushed; i++) {
    appstate->uiCache->textColorStack.pop_back();
  }
  for (int i = 0; i < textSizePushed; i++) {
    appstate->uiCache->textSizeStack.pop_back();
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
        appstate->uiCache->textColorStack.push_back(COLORS.at(value));
        textColorPushed++;
      }
      else if (FONT_SIZES.contains(value)) {
        appstate->uiCache->textSizeStack.push_back(FONT_SIZES.at(value));
        textSizePushed++;
      }
    }
  }

  Color textColor = COLORS["black"];
  if (appstate->uiCache->textColorStack.size() > 0) {
    textColor = appstate->uiCache->textColorStack.back();
  }

  int textsize = FONT_SIZES["base"];
  if (appstate->uiCache->textSizeStack.size() > 0) {
    textsize = appstate->uiCache->textSizeStack.back();
  }

  CLAY_TEXT(ClayString(appstate, content),
            CLAY_TEXT_CONFIG({
                .textColor = textColor,
                .fontSize = textsize,
            }));

  for (int i = 0; i < textColorPushed; i++) {
    appstate->uiCache->textColorStack.pop_back();
  }
  for (int i = 0; i < textSizePushed; i++) {
    appstate->uiCache->textSizeStack.pop_back();
  }
}