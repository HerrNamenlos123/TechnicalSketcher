
#include "../std.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

struct UICache {
  List<Color> textColorStack;
  List<int> textSizeStack;
  std::deque<String> stringCache;
  List<SDL_Texture*> pages;
};
