#include "GlfwWebGPURenderingBackend.h"

namespace Walnut
{
    void GlfwWebGPURenderingBackend::Init(GLFWwindow *windowHandle)
    {
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
    void GlfwWebGPURenderingBackend::StartImGuiFrame(const std::function<void()> &applicationMenubarCallback, const std::function<void()> &applicationUIRenderingCallback)
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
        return nullptr;
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