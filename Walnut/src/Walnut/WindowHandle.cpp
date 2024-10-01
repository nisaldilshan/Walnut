#include "WindowHandle.h"

#ifdef USE_SDL
#include <SDL2/SDL_platform.h>
#if defined(__ANDROID__) || defined(__iOS__)
#include <SDL2/SDL_main.h>
#else
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#else

#if defined(__EMSCRIPTEN__)
#define GLFW_INCLUDE_ES3
#else
#define GLFW_INCLUDE_NONE
#endif

#if (RENDERER_BACKEND == 2) // 2 -> Vulkan
#define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>

#endif