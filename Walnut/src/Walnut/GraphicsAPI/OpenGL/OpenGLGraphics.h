#pragma once

class GLFWwindow;

namespace GraphicsAPI
{
    class OpenGL
    {
    public:
        static void SetupOpenGL(GLFWwindow* windowHandle);
        static void SetupViewport(int width, int height);
    };
}