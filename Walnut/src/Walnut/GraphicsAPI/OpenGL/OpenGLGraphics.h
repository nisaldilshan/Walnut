#pragma once

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

// The browser (WebGL 2), Android and iOS provide OpenGL ES 3 through the platform's own header and library,
// so they need no loader.
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#define WALNUT_OPENGL_ES
#include <GLES3/gl3.h>
#elif defined(__APPLE__) && TARGET_OS_IPHONE
#define WALNUT_OPENGL_ES
// UIKit has no default framebuffer: SDL renders to a framebuffer object of its own
#define WALNUT_OPENGL_UIKIT
#include <OpenGLES/ES3/gl.h>
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