
#include "../shared/shared.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

struct UICache {
  List<Color> textColorStack;
  List<int> textSizeStack;
  List<SDL_Texture*> pages;
};
