#pragma once

#ifdef USE_SDL
#include <SDL3/SDL.h>
#else
#include <GLFW/glfw3.h>
#endif


#ifdef USE_SDL
typedef SDL_Window WalnutWindowHandleType;
#else
typedef GLFWwindow WalnutWindowHandleType;
#endif


