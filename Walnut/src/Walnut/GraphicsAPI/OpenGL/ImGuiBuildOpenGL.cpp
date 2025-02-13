#include <imgui.cpp>
#include <imgui_demo.cpp>
#include <imgui_draw.cpp>
#include <imgui_tables.cpp>
#include <imgui_widgets.cpp>

#if defined(__ANDROID__) || defined(__iOS__)
#define IMGUI_IMPL_OPENGL_ES3
#endif
#include <imgui_impl_opengl3.cpp>

#ifdef USE_SDL
#include <imgui_impl_sdl2.cpp>
#else
#include <imgui_impl_glfw.cpp>
#endif
