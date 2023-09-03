#pragma once

#include <imgui_impl_glfw.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/emscripten.h>
#define GLFW_INCLUDE_ES3
#else
#define GLFW_INCLUDE_NONE
#endif

#include <GLFW/glfw3.h>

namespace GraphicsAPI
{
    class OpenGL
    {
    public:
        static void SetupOpenGL(GLFWwindow *windowHandle);
        static void SetupViewport(int width, int height);
    };
}