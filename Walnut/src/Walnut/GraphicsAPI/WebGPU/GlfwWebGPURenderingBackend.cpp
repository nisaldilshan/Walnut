#include "WebGPURenderingBackend.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "WebGPUGraphics.h"

#include "WebGPUImageRenderPipeline.h"
#include <Walnut/Image.h>
#include "WebGPUImage.h"

#include <iostream>

namespace Walnut
{

    void GlfwWebGPURenderingBackend::CreateImGuiPipeline()
    {
        ImGui_ImplGlfw_InitForOther(m_windowHandle, true);

        ImGui_ImplWGPU_InitInfo initInfo{};
        initInfo.Device = GraphicsAPI::WebGPU::GetDevice();
        initInfo.NumFramesInFlight = 3;
        initInfo.RenderTargetFormat = GraphicsAPI::WebGPU::GetSwapChainFormat();
        ImGui_ImplWGPU_Init(&initInfo);
    }

    void GlfwWebGPURenderingBackend::StartImGuiFrame()
    {
        ImGui_ImplWGPU_NewFrame();
		ImGui_ImplGlfw_NewFrame();
    }

    void GlfwWebGPURenderingBackend::DestroyImGuiPipeline()
    {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        if (GraphicsAPI::WebGPU::GetSurface())
        {
            GraphicsAPI::WebGPU::GetSurface().release();
        }
    }

}