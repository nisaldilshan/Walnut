

//namespace Walnut {

#ifdef USE_SDL
class SDL_Window;
typedef SDL_Window WindowHandleType;
#else
class GLFWwindow;
typedef GLFWwindow WindowHandleType;
#endif

//