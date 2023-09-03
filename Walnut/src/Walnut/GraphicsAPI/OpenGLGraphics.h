#pragma once

#include <imgui_impl_glfw.h>

#define GLFW_INCLUDE_NONE
//#define GLFW_INCLUDE_GLEXT
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