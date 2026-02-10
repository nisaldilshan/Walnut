#pragma once

#ifdef USE_SDL
class SDL_Window;
typedef SDL_Window WalnutWindowHandleType;
#else
class GLFWwindow;
typedef GLFWwindow WalnutWindowHandleType;
#endif


