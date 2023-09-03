#include "OpenGLGraphics.h"

#include <iostream>
#include <glad/glad.h>

namespace GraphicsAPI
{

void OpenGLMessageCallback(
    unsigned source,
    unsigned type,
    unsigned id,
    unsigned severity,
    int length,
    const char* message,
    const void* userParam)
{
    std::cout << message << std::endl;
    // switch (severity)
    // {
    // 	case GL_DEBUG_SEVERITY_HIGH:         std::cout << message << std::endl; return;
    // 	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << message << std::endl; return;
    // 	case GL_DEBUG_SEVERITY_LOW:          std::cout << message << std::endl; return;
    // 	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << message << std::endl; return;
    // }
    
}

const char * GetGLErrorStr(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:          return "No error";
    case GL_INVALID_ENUM:      return "Invalid enum";
    case GL_INVALID_VALUE:     return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_OUT_OF_MEMORY:     return "Out of memory";
    default:                   return "Unknown error";
    }
}

inline void CheckGLError()
{
    while (true)
    {
        const GLenum err = glGetError();
        if (GL_NO_ERROR == err)
            break;

        std::cout << "GL Error: " << GetGLErrorStr(err) << std::endl;
    }
}

void OpenGL::SetupOpenGL(GLFWwindow *windowHandle)
{
    GLenum internalFormat = GL_RGBA8;

    glfwMakeContextCurrent(windowHandle);
    int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    gladLoadGL();
    if (!status)
    {
        std::cout << "Failed to initialize Glad!" << std::endl;
        return;
    }
    std::cout << "Graphics Info:" << std::endl;
    std::cout << "\tVendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "\tVersion: " << glGetString(GL_VERSION) << std::endl;

    //glEnable(GL_DEBUG_OUTPUT);
    //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    //glDebugMessageCallback(OpenGLMessageCallback, nullptr);		
    //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    //CheckGLError();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
}

void OpenGL::SetupViewport(int width, int height)
{
    int x = 0;
    int y = 0;
    glViewport(x, y, width, height);
}

}
