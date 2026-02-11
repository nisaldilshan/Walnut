#pragma once

#ifdef USE_SDL
struct SDL_Window;
typedef SDL_Window WalnutWindowHandleType;
#else
struct GLFWwindow;
typedef GLFWwindow WalnutWindowHandleType;
#endif


