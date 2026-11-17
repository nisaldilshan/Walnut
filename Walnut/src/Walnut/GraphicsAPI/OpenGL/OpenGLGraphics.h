#pragma once

// The browser (WebGL 2) and Android provide OpenGL ES 3 through the platform's own header and library,
// so they need no loader.
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#define WALNUT_OPENGL_ES
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

namespace GraphicsAPI
{
    class OpenGL
    {
    public:
        static void SetupOpenGL(void* loaderFunc);
        static void SetupViewport(int width, int height);
    };
}