#pragma once

namespace GraphicsAPI
{
    class OpenGL
    {
    public:
        static void SetupOpenGL(void* loaderFunc);
        static void SetupViewport(int width, int height);
    };
}