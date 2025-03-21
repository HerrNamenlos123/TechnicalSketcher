
#include "../shared/app.h"
#include "uiCache.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_system.h>

List<Pair<String, Color>> COLORS;
List<Pair<String, int>> FONT_SIZES;

List<String> split(Arena& arena, String s, String delimiter)
{
  size_t pos_start = 0, pos_end, delim_len = delimiter.length;
  List<String> res;

  while ((pos_end = s.find(delimiter, pos_start)) != String::npos) {
    String token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push(arena, token);
  }

  res.push(arena, s.substr(pos_start));
  return res;
}

Clay_String ClayString(String str)
{
  return Clay_String {
    .length = (int32_t)str.length,
    .chars = str.data,
  };
}

Pair<String, String> parseClass(Arena& arena, String classString)
{
  auto elements = split(arena, classString, ":"_s);
  if (elements.length == 1) {
    return { ""_s, elements[0] };
  } else if (elements.length == 2) {
    return { elements[0], elements[1] };
  } else {
    return { ""_s, ""_s };
  }
}

Optional<Color> parseBgClass(String cls)
{
  if (cls.startsWith("bg-["_s)) {
    return Color(cls.substr(4, cls.length - 5));
  } else if (cls.startsWith("bg-"_s)) {
    auto value = cls.substr(3, cls.length - 3);
    for (auto [name, color] : COLORS) {
      if (name == value) {
        return color;
      }
    }
  }
  return {};
}

void div(App* app, String classString, void (*cb)(App* app), Optional<SDL_Texture*> image = {})
{
  List<String> classes = split(app->frameArena, classString, " "_s);
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
    const auto [modifier, cmd] = parseClass(app->frameArena, classString);

    if (cmd == "w-full"_s) {
      width = CLAY_SIZING_GROW(0);
    } else if (cmd == "h-full"_s) {
      height = CLAY_SIZING_GROW(0);
    } else if (cmd.startsWith("w-["_s)) {
      auto unit = cmd.substr(3, cmd.length - 4);
      if (unit.endsWith("px"_s)) {
        if (auto value = strToInt(unit.substr(0, unit.length - 2))) {
          width = CLAY_SIZING_FIXED((float)value.unwrap());
        }
      }
    } else if (cmd.startsWith("h-["_s)) {
      auto unit = cmd.substr(3, cmd.length - 4);
      if (unit.endsWith("px"_s)) {
        if (auto value = strToInt(unit.substr(0, unit.length - 2))) {
          height = CLAY_SIZING_FIXED((float)value.unwrap());
        }
      }
    } else if (cmd.startsWith("id-"_s)) {
      auto value = cmd.substr(3, cmd.length - 3);
      id = CLAY_SIDI(ClayString(value), 0);
    } else if (cmd.startsWith("p-["_s)) {
      auto unit = cmd.substr(3, cmd.length - 4);
      if (unit.endsWith("px"_s)) {
        if (auto value = strToInt(unit.substr(0, unit.length - 2))) {
          padding = CLAY_PADDING_ALL((uint16_t)value.unwrap());
        }
      }
    } else if (cmd == "col"_s) {
      layoutDirection = CLAY_TOP_TO_BOTTOM;
      childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_TOP };
    } else if (auto col = parseBgClass(cmd)) {
      backgroundColor = *col;
    } else if (cmd.startsWith("text-"_s)) {
      auto value = cmd.substr(5, cmd.length - 5);
      for (auto [name, color] : COLORS) {
        if (name == value) {
          app->uiCache->textColorStack.push(app->frameArena, color);
          textColorPushed++;
        }
      }
      for (auto [sizeName, sizePx] : FONT_SIZES) {
        if (sizeName == value) {
          app->uiCache->textSizeStack.push(app->frameArena, sizePx);
          textColorPushed++;
        }
      }
    }
  }

  Color textColor;
  for (auto& [name, color] : COLORS) {
    if (name == "black"_s) {
      textColor = color;
    }
  }
  if (app->uiCache->textColorStack.length > 0) {
    textColor = app->uiCache->textColorStack.back();
  }

  int textsize = 0;
  for (auto& [name, size] : FONT_SIZES) {
    if (name == "base"_s) {
      textsize = size;
    }
  }
  if (app->uiCache->textSizeStack.length > 0) {
    textsize = app->uiCache->textSizeStack.back();
  }

  Clay_ImageElementConfig imageConfig = {};
  if (image && *image) {
    auto texture = app->mainDocumentRenderTexture;
    imageConfig = { .imageData = *image, .sourceDimensions = { (float)texture->w, (float)texture->h } };
  }

  CLAY({
      .id = id,
      .layout = {
          .sizing = { .width = width, .height = height },
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
      cb(app);
    }
  }

  for (int i = 0; i < textColorPushed; i++) {
    app->uiCache->textColorStack.pop();
  }
  for (int i = 0; i < textSizePushed; i++) {
    app->uiCache->textSizeStack.pop();
  }
}

void text(App* app, String classString, String content)
{
  List<String> classes = split(app->frameArena, classString, " "_s);
  Clay_SizingAxis width;
  Clay_SizingAxis height;
  Clay_Padding padding;
  Color backgroundColor;

  size_t textColorPushed = 0;
  size_t textSizePushed = 0;

  for (const auto& classString : classes) {
    const auto [modifier, cmd] = parseClass(app->frameArena, classString);

    if (cmd == "w-full"_s) {
      width = CLAY_SIZING_GROW(0);
    } else if (cmd == "h-full"_s) {
      height = CLAY_SIZING_GROW(0);
    } else if (cmd.startsWith("p-["_s)) {
      auto unit = cmd.substr(3, cmd.length - 4);
      if (unit.endsWith("px"_s)) {
        try {
          auto value = strToInt(unit.substr(0, unit.length - 2));
          padding = CLAY_PADDING_ALL((uint16_t)value.unwrap());
        } catch (...) {
        }
      }
    } else if (cmd.startsWith("bg-"_s)) {
      auto value = cmd.substr(3, cmd.length - 3);
      for (auto& [name, color] : COLORS) {
        if (name == value) {
          backgroundColor = color;
        }
      }
    } else if (cmd.startsWith("text-"_s)) {
      auto value = cmd.substr(5, cmd.length - 5);
      for (auto [name, color] : COLORS) {
        if (name == value) {
          app->uiCache->textColorStack.push(app->frameArena, color);
          textColorPushed++;
        }
      }
      for (auto [sizeName, sizePx] : FONT_SIZES) {
        if (sizeName == value) {
          app->uiCache->textSizeStack.push(app->frameArena, sizePx);
          textColorPushed++;
        }
      }
    }
  }

  Color textColor;
  for (auto& [name, color] : COLORS) {
    if (name == "black"_s) {
      textColor = color;
    }
  }
  if (app->uiCache->textColorStack.length > 0) {
    textColor = app->uiCache->textColorStack.back();
  }

  int textsize = 0;
  for (auto& [name, size] : FONT_SIZES) {
    if (name == "base"_s) {
      textsize = size;
    }
  }
  if (app->uiCache->textSizeStack.length > 0) {
    textsize = app->uiCache->textSizeStack.back();
  }

  CLAY_TEXT(ClayString(content),
      CLAY_TEXT_CONFIG({
          .textColor = textColor,
          .fontSize = (uint16_t)textsize,
      }));

  for (int i = 0; i < textColorPushed; i++) {
    app->uiCache->textColorStack.pop();
  }
  for (int i = 0; i < textSizePushed; i++) {
    app->uiCache->textSizeStack.pop();
  }
}