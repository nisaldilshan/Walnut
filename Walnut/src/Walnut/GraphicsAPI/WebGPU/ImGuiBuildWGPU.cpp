#include <imgui.cpp>
#include <imgui_demo.cpp>
#include <imgui_draw.cpp>
#include <imgui_tables.cpp>
#include <imgui_widgets.cpp>

// #define IMGUI_IMPL_WEBGPU_BACKEND_DAWN
// #include <imgui_impl_wgpu.cpp>

#ifdef USE_SDL
#include <imgui_impl_sdl3.cpp>
#else
#include <imgui_impl_glfw.cpp>
#endif
