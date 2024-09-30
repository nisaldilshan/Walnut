#pragma once

#ifdef USE_SDL
#include <SDL2/SDL_platform.h>
#if defined(__ANDROID__) || defined(__iOS__)
#include <SDL2/SDL_main.h>
#else
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#else
#include <GLFW/glfw3.h>
#endif

namespace Walnut {

#ifdef USE_SDL
typedef SDL_Window WindowHandleType;
#else
typedef GLFWwindow WindowHandleType;
#endif

}