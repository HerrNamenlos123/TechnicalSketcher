
#include "../shared/app.h"
#include "clay/clay_renderer.h"
#include "uiCache.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_system.h>

struct ImageData {
  GLuint texture;
  float width;
  float height;
};

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

Optional<Color> parseBgClass(App* app, String cls)
{
  if (cls.startsWith("bg-["_s)) {
    return Color(cls.substr(4, cls.length - 5));
  } else if (cls.startsWith("bg-"_s)) {
    auto value = cls.substr(3, cls.length - 3);
    for (auto [name, color] : app->colors) {
      if (name == value) {
        return color;
      }
    }
  }
  return {};
}

struct DivStyle {
  Optional<String> id;
  Optional<int> paddingLeft;
  Optional<int> paddingRight;
  Optional<int> paddingTop;
  Optional<int> paddingBottom;
  Optional<String> width;
  Optional<String> height;
  Optional<Color> backgroundColor;
  Optional<Color> fontColor;
  Optional<int> fontSize;
  Optional<int> fontWeight;
  Optional<String> alignHorizontal;
  Optional<String> alignVertical;
  Optional<String> layoutDirection;

  struct FloatingDiv {
    Optional<String> left;
    Optional<String> right;
    Optional<String> attachTo;
    Optional<String> attachToId;
    Optional<Vec2> offset;
  };
  Optional<FloatingDiv> floating;
};

template <typename TFunc> void div(App* app, DivStyle style, TFunc&& cb, Optional<ImageData> image = {})
{
  Clay_SizingAxis width = {};
  if (style.width) {
    auto unit = *style.width;
    if (unit == "full"_s || unit == "100%"_s) {
      width = CLAY_SIZING_GROW(0);
    } else if (unit.endsWith("px"_s)) {
      if (auto value = strToInt(unit.substr(0, unit.length - 2))) {
        width = CLAY_SIZING_FIXED((float)value.value());
      }
    }
  }

  Clay_SizingAxis height = {};
  if (style.height) {
    auto unit = *style.height;
    if (unit == "full"_s || unit == "100%"_s) {
      height = CLAY_SIZING_GROW(0);
    } else if (unit.endsWith("px"_s)) {
      if (auto value = strToInt(unit.substr(0, unit.length - 2))) {
        height = CLAY_SIZING_FIXED((float)value.value());
      }
    }
  }

  Clay_ElementId id = {};
  if (style.id) {
    auto idValue = *style.id;
    id = CLAY_SIDI(ClayString(idValue), 0);
  }

  Clay_Padding padding = (Clay_Padding) {
    .left = style.paddingLeft.value_or(0),
    .right = style.paddingRight.value_or(0),
    .top = style.paddingTop.value_or(0),
    .bottom = style.paddingBottom.value_or(0),
  };

  Color backgroundColor = {};
  if (style.backgroundColor) {
    backgroundColor = *style.backgroundColor;
  }

  Clay_ChildAlignment childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_TOP };
  if (style.alignHorizontal) {
    if (*style.alignHorizontal == "center"_s) {
      childAlignment.x = CLAY_ALIGN_X_CENTER;
    } else if (*style.alignHorizontal == "left"_s) {
      childAlignment.x = CLAY_ALIGN_X_LEFT;
    } else if (*style.alignHorizontal == "right"_s) {
      childAlignment.x = CLAY_ALIGN_X_RIGHT;
    }
  }
  if (style.alignVertical) {
    if (*style.alignVertical == "center"_s) {
      childAlignment.y = CLAY_ALIGN_Y_CENTER;
    } else if (*style.alignVertical == "top"_s) {
      childAlignment.y = CLAY_ALIGN_Y_TOP;
    } else if (*style.alignVertical == "bottom"_s) {
      childAlignment.y = CLAY_ALIGN_Y_BOTTOM;
    }
  }

  Clay_LayoutDirection layoutDirection = CLAY_LEFT_TO_RIGHT;
  if (style.layoutDirection) {
    if (*style.layoutDirection == "right"_s || *style.layoutDirection == "row"_s) {
      layoutDirection = CLAY_LEFT_TO_RIGHT;
    } else if (*style.layoutDirection == "down"_s || *style.layoutDirection == "column"_s
        || *style.layoutDirection == "col"_s) {
      layoutDirection = CLAY_TOP_TO_BOTTOM;
    }
  }

  Clay_FloatingElementConfig floating = {};
  if (style.floating) {
    if (style.floating->attachTo) {
      if (*style.floating->attachTo == "parent") {
        floating.attachTo = CLAY_ATTACH_TO_PARENT;
      } else if (*style.floating->attachTo == "root") {
        floating.attachTo = CLAY_ATTACH_TO_ROOT;
      } else if (*style.floating->attachTo == "id") {
        floating.attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID;
        if (style.floating->attachToId) {
          floating.parentId = CLAY_SIDI(ClayString(style.floating->attachToId->c_str(app->frameArena)), 0).id;
        }
      }
    }
    if (style.floating->offset) {
      floating.offset = (Clay_Vector2) { style.floating->offset->x, style.floating->offset->y };
    }
  }

  size_t textColorPushed = 0;
  size_t textSizePushed = 0;

  if (style.fontColor) {
    app->uiCache->textColorStack.push(app->frameArena, *style.fontColor);
    textColorPushed++;
  }
  if (style.fontSize) {
    app->uiCache->textSizeStack.push(app->frameArena, *style.fontSize);
    textSizePushed++;
  }

  Color textColor;
  for (auto& [name, color] : app->colors) {
    if (name == "black"_s) {
      textColor = color;
    }
  }
  if (app->uiCache->textColorStack.length > 0) {
    textColor = app->uiCache->textColorStack.back();
  }

  int textsize = 0;
  for (auto& [name, size] : app->fonts) {
    if (name == "base"_s) {
      textsize = size;
    }
  }
  if (app->uiCache->textSizeStack.length > 0) {
    textsize = app->uiCache->textSizeStack.back();
  }

  Clay_ImageElementConfig imageConfig = {};
  if (image) {
    GLuint* texture = app->frameArena.allocate<GLuint>();
    *texture = image->texture;
    imageConfig = { .imageData = texture, .sourceDimensions = { (float)image->width, (float)image->height } };
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
      .floating = floating,
  })
  {
    cb(app);
  }

  for (int i = 0; i < textColorPushed; i++) {
    app->uiCache->textColorStack.pop();
  }
  for (int i = 0; i < textSizePushed; i++) {
    app->uiCache->textSizeStack.pop();
  }
}

// template <typename TFunc> void div(App* app, String classString, TFunc&& cb, Optional<ImageData> image = {})
// {
//   List<String> classes = split(app->frameArena, classString, " "_s);
//   Clay_SizingAxis width = {};
//   Clay_SizingAxis height = {};
//   Clay_Padding padding = {};
//   Clay_LayoutDirection layoutDirection = CLAY_LEFT_TO_RIGHT;
//   Clay_ChildAlignment childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_TOP };
//   Clay_ElementId id = {};
//   Color backgroundColor = {};

//   size_t textColorPushed = 0;
//   size_t textSizePushed = 0;

//   for (const auto& classString : classes) {
//     const auto [modifier, cmd] = parseClass(app->frameArena, classString);

//     if (cmd == "w-full"_s) {
//       width = CLAY_SIZING_GROW(0);
//     } else if (cmd == "h-full"_s) {
//       height = CLAY_SIZING_GROW(0);
//     } else if (cmd.startsWith("w-["_s)) {
//       auto unit = cmd.substr(3, cmd.length - 4);
//       if (unit.endsWith("px"_s)) {
//         if (auto value = strToInt(unit.substr(0, unit.length - 2))) {
//           width = CLAY_SIZING_FIXED((float)value.unwrap());
//         }
//       }
//     } else if (cmd.startsWith("h-["_s)) {
//       auto unit = cmd.substr(3, cmd.length - 4);
//       if (unit.endsWith("px"_s)) {
//         if (auto value = strToInt(unit.substr(0, unit.length - 2))) {
//           height = CLAY_SIZING_FIXED((float)value.unwrap());
//         }
//       }
//     } else if (cmd.startsWith("id-["_s)) {
//       auto value = cmd.substr(4, cmd.length - 5);
//       id = CLAY_SIDI(ClayString(value), 0);
//     } else if (cmd.startsWith("p-["_s)) {
//       auto unit = cmd.substr(3, cmd.length - 4);
//       if (unit.endsWith("px"_s)) {
//         if (auto value = strToInt(unit.substr(0, unit.length - 2))) {
//           padding = CLAY_PADDING_ALL((uint16_t)value.unwrap());
//         }
//       }
//     } else if (cmd == "col"_s) {
//       layoutDirection = CLAY_TOP_TO_BOTTOM;
//       childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_TOP };
//     } else if (auto col = parseBgClass(app, cmd)) {
//       backgroundColor = *col;
//     } else if (cmd.startsWith("text-"_s)) {
//       auto value = cmd.substr(5, cmd.length - 5);
//       for (auto [name, color] : app->colors) {
//         if (name == value) {
//           app->uiCache->textColorStack.push(app->frameArena, color);
//           textColorPushed++;
//         }
//       }
//       for (auto [sizeName, sizePx] : app->fonts) {
//         if (sizeName == value) {
//           app->uiCache->textSizeStack.push(app->frameArena, sizePx);
//           textColorPushed++;
//         }
//       }
//     }
//   }

//   Color textColor;
//   for (auto& [name, color] : app->colors) {
//     if (name == "black"_s) {
//       textColor = color;
//     }
//   }
//   if (app->uiCache->textColorStack.length > 0) {
//     textColor = app->uiCache->textColorStack.back();
//   }

//   int textsize = 0;
//   for (auto& [name, size] : app->fonts) {
//     if (name == "base"_s) {
//       textsize = size;
//     }
//   }
//   if (app->uiCache->textSizeStack.length > 0) {
//     textsize = app->uiCache->textSizeStack.back();
//   }

//   Clay_ImageElementConfig imageConfig = {};
//   if (image) {
//     GLuint* texture = app->frameArena.allocate<GLuint>();
//     *texture = image->texture;
//     imageConfig = { .imageData = texture, .sourceDimensions = { (float)image->width, (float)image->height } };
//   }

//   CLAY({
//       .id = id,
//       .layout = {
//           .sizing = { .width = width, .height = height },
//           .padding = CLAY_PADDING_ALL(0),
//           .childGap = 0,
//           .childAlignment = childAlignment,
//           .layoutDirection = layoutDirection,
//       },
//       .backgroundColor = backgroundColor,
//       .image = imageConfig,
//   })
//   {
//     cb(app);
//   }

//   for (int i = 0; i < textColorPushed; i++) {
//     app->uiCache->textColorStack.pop();
//   }
//   for (int i = 0; i < textSizePushed; i++) {
//     app->uiCache->textSizeStack.pop();
//   }
// }

// void text(App* app, String classString, String content)
// {
//   List<String> classes = split(app->frameArena, classString, " "_s);
//   Clay_SizingAxis width;
//   Clay_SizingAxis height;
//   Clay_Padding padding;
//   Color backgroundColor;

//   size_t textColorPushed = 0;
//   size_t textSizePushed = 0;

//   for (const auto& classString : classes) {
//     const auto [modifier, cmd] = parseClass(app->frameArena, classString);

//     if (cmd == "w-full"_s) {
//       width = CLAY_SIZING_GROW(0);
//     } else if (cmd == "h-full"_s) {
//       height = CLAY_SIZING_GROW(0);
//     } else if (cmd.startsWith("p-["_s)) {
//       auto unit = cmd.substr(3, cmd.length - 4);
//       if (unit.endsWith("px"_s)) {
//         try {
//           auto value = strToInt(unit.substr(0, unit.length - 2));
//           padding = CLAY_PADDING_ALL((uint16_t)value.value());
//         } catch (...) {
//         }
//       }
//     } else if (cmd.startsWith("bg-"_s)) {
//       auto value = cmd.substr(3, cmd.length - 3);
//       for (auto& [name, color] : app->colors) {
//         if (name == value) {
//           backgroundColor = color;
//         }
//       }
//     } else if (cmd.startsWith("text-"_s)) {
//       auto value = cmd.substr(5, cmd.length - 5);
//       for (auto [name, color] : app->colors) {
//         if (name == value) {
//           app->uiCache->textColorStack.push(app->frameArena, color);
//           textColorPushed++;
//         }
//       }
//       for (auto [sizeName, sizePx] : app->fonts) {
//         if (sizeName == value) {
//           app->uiCache->textSizeStack.push(app->frameArena, sizePx);
//           textColorPushed++;
//         }
//       }
//     }
//   }

//   Color textColor;
//   for (auto& [name, color] : app->colors) {
//     if (name == "black"_s) {
//       textColor = color;
//     }
//   }
//   if (app->uiCache->textColorStack.length > 0) {
//     textColor = app->uiCache->textColorStack.back();
//   }

//   int textsize = 0;
//   for (auto& [name, size] : app->fonts) {
//     if (name == "base"_s) {
//       textsize = size;
//     }
//   }
//   if (app->uiCache->textSizeStack.length > 0) {
//     textsize = app->uiCache->textSizeStack.back();
//   }

//   CLAY_TEXT(ClayString(content),
//       CLAY_TEXT_CONFIG({
//           .textColor = textColor,
//           .fontId = 0,
//           .fontSize = (uint16_t)textsize,
//           .letterSpacing = 0,
//       }));
//   // print("Font: {}", content);

//   for (int i = 0; i < textColorPushed; i++) {
//     app->uiCache->textColorStack.pop();
//   }
//   for (int i = 0; i < textSizePushed; i++) {
//     app->uiCache->textSizeStack.pop();
//   }
// }

struct TextStyle {
  Optional<Color> fontColor;
  Optional<int> fontSize;
  // Optional<int> fontWeight;
  Optional<String> textAlign;
};

void text(App* app, TextStyle style, String content)
{
  Clay_TextAlignment textAlignment = CLAY_TEXT_ALIGN_LEFT;
  if (style.textAlign) {
    if (*style.textAlign == "center"_s) {
      textAlignment = CLAY_TEXT_ALIGN_CENTER;
    } else if (*style.textAlign == "left"_s) {
      textAlignment = CLAY_TEXT_ALIGN_LEFT;
    } else if (*style.textAlign == "right"_s) {
      textAlignment = CLAY_TEXT_ALIGN_RIGHT;
    }
  }

  size_t textColorPushed = 0;
  size_t textSizePushed = 0;

  if (style.fontColor) {
    app->uiCache->textColorStack.push(app->frameArena, *style.fontColor);
    textColorPushed++;
  }
  if (style.fontSize) {
    app->uiCache->textSizeStack.push(app->frameArena, *style.fontSize);
    textSizePushed++;
  }

  Color textColor;
  for (auto& [name, color] : app->colors) {
    if (name == "black"_s) {
      textColor = color;
    }
  }
  if (app->uiCache->textColorStack.length > 0) {
    textColor = app->uiCache->textColorStack.back();
  }

  int textsize = 0;
  for (auto& [name, size] : app->fonts) {
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
          .fontId = 0,
          .fontSize = (uint16_t)textsize,
          .letterSpacing = 0,
          .lineHeight = 0,
          .textAlignment = textAlignment,
      }));

  for (int i = 0; i < textColorPushed; i++) {
    app->uiCache->textColorStack.pop();
  }
  for (int i = 0; i < textSizePushed; i++) {
    app->uiCache->textSizeStack.pop();
  }
}