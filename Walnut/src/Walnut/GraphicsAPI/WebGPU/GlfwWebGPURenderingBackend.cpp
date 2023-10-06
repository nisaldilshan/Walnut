#include "GlfwWebGPURenderingBackend.h"

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
        ImGui_ImplGlfw_InitForOther(m_windowHandle, true);
        //ImGui_ImplWGPU_Init(m_wgpu_device, 3, m_swapChainFormat, m_depthTextureFormat);
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