#pragma once

#ifdef USE_SDL
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#else
#endif

namespace Walnut {

#ifdef USE_SDL
typedef SDL_Window WindowHandleType;
#else
//typedef GLFWwindow WindowHandleType;
#endif

}