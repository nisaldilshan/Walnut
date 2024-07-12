#pragma once

typedef void* (* LoaderFuncType)(const char *);

namespace GraphicsAPI
{
    class OpenGL
    {
    public:
        static void SetupOpenGL(LoaderFuncType loaderFunc);
        static void SetupViewport(int width, int height);
    };
}