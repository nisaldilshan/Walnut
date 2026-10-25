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

#ifdef __EMSCRIPTEN__
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace Walnut
{
#ifdef __EMSCRIPTEN__
    // The page's canvas fills the browser window, so the GLFW window follows the browser window's size.
    // ImGui's own resize callback reads the canvas CSS size instead, which Emscripten's GLFW pins to the
    // GLFW window size when GLFW_SCALE_TO_MONITOR is set, so the window would never change size.
    static EM_BOOL ResizeToBrowserWindow(int /*eventType*/, const EmscriptenUiEvent* event, void* window)
    {
        glfwSetWindowSize(static_cast<GLFWwindow*>(window), event->windowInnerWidth, event->windowInnerHeight);
        return EM_FALSE;
    }
#endif

    void WebGPURenderingBackend::CreateImGuiPipeline()
    {
        ImGui_ImplGlfw_InitForOther(m_windowHandle, true);
#ifdef __EMSCRIPTEN__
        ImGui_ImplGlfw_InstallEmscriptenCallbacks(m_windowHandle, "#canvas");
        // Registered after ImGui's resize callback, so it runs last and its size wins.
        emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, m_windowHandle, false, ResizeToBrowserWindow);
        glfwSetWindowSize(m_windowHandle, EM_ASM_INT({ return window.innerWidth; }), EM_ASM_INT({ return window.innerHeight; }));
#endif

        ImGui_ImplWGPU_InitInfo initInfo{};
        initInfo.Device = GraphicsAPI::WebGPU::GetDevice();
        initInfo.NumFramesInFlight = 3;
        initInfo.RenderTargetFormat = GraphicsAPI::WebGPU::GetSwapChainFormat();
        ImGui_ImplWGPU_Init(&initInfo);
    }

    void WebGPURenderingBackend::StartImGuiFrame()
    {
        ImGui_ImplWGPU_NewFrame();
		ImGui_ImplGlfw_NewFrame();
    }

    void WebGPURenderingBackend::DestroyImGuiPipeline()
    {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        if (GraphicsAPI::WebGPU::GetSurface())
        {
            GraphicsAPI::WebGPU::GetSurface().release();
        }
    }

}