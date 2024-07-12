#pragma once

#ifdef USE_SDL
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