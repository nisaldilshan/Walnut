#include "GlfwWebGPURenderingBackend.h"

#include <iostream>
//#include <imgui_impl_wgpu.h>


//#include "../../../../../vendor/glfw3webgpu/glfw3webgpu.h"


namespace Walnut
{
    void GlfwWebGPURenderingBackend::Init(GLFWwindow *windowHandle)
    {
        m_windowHandle = windowHandle;



    }
    void GlfwWebGPURenderingBackend::SetupGraphicsAPI()
    {
    }

    void GlfwWebGPURenderingBackend::SetupWindow(int width, int height)
    {
        
    }
    bool GlfwWebGPURenderingBackend::NeedToResizeWindow()
    {
        return false;
    }
    void GlfwWebGPURenderingBackend::ResizeWindow(int width, int height)
    {
    }
    void GlfwWebGPURenderingBackend::ConfigureImGui()
    {

    }

    void GlfwWebGPURenderingBackend::StartImGuiFrame()
    {
        
    }

    void GlfwWebGPURenderingBackend::UploadFonts()
    {
    }

    void GlfwWebGPURenderingBackend::FrameRender(ImDrawData *draw_data)
    {

    }

    void GlfwWebGPURenderingBackend::FramePresent()
    {
        
    }

    GLFWwindow *GlfwWebGPURenderingBackend::GetWindowHandle()
    {
        return m_windowHandle;
    }

    void GlfwWebGPURenderingBackend::Shutdown()
    {
    }
    void GlfwWebGPURenderingBackend::Cleanup()
    {
    }
    void GlfwWebGPURenderingBackend::SetClearColor(ImVec4 color)
    {
    }
}