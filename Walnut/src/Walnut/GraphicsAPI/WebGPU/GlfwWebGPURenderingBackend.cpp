#include "GlfwWebGPURenderingBackend.h"

#include <iostream>
#include <imgui_impl_wgpu.h>

namespace Walnut
{
    void GlfwWebGPURenderingBackend::Init(GLFWwindow *windowHandle)
    {
        m_windowHandle = windowHandle;

        DawnProcTable procs(dawn_native::GetProcs());
        dawnProcSetProcs(&procs);

        // 1. create a descriptor
        WGPUInstanceDescriptor desc = {};
        desc.nextInChain = nullptr;

        // 2. create the instance using this descriptor
        WGPUInstance instance = wgpuCreateInstance(&desc);

        // 3. We can check whether there is actually an instance created
        if (!instance) {
            std::cerr << "Could not initialize WebGPU!" << std::endl;
            return;
        }

        // 4. Display the object (WGPUInstance is a simple pointer, it may be
        // copied around without worrying about its size).
        std::cout << "WGPU instance: " << instance << std::endl;


        //m_wgpuDevice = emscripten_webgpu_get_device();


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
        ImGui_ImplWGPU_Init(m_wgpuDevice, 3, m_wgpuPreferredFormat, WGPUTextureFormat_Undefined);
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