#include "GlfwWebGPURenderingBackend.h"

#include <iostream>
#include <imgui_impl_wgpu.h>



#include <dawn/webgpu_cpp.h>
#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include "../../../../../vendor/glfw3webgpu/glfw3webgpu.h"

wgpu::Instance m_wgpuInstance;
std::unique_ptr<dawn::native::Instance> m_wgpuNativeInstance;
wgpu::Device m_wgpuDevice;
WGPUTextureFormat m_wgpuPreferredFormat = WGPUTextureFormat_RGBA8Unorm;
wgpu::SwapChain m_swapChain;
wgpu::CommandEncoder m_commandEncoder;

// Return backend select priority, smaller number means higher priority.
int GetBackendPriority(wgpu::BackendType t) {
    switch (t) {
        case wgpu::BackendType::Null:
            return 9999;
        case wgpu::BackendType::D3D12:
        case wgpu::BackendType::Metal:
        case wgpu::BackendType::Vulkan:
            return 0;
        case wgpu::BackendType::WebGPU:
            return 5;
        case wgpu::BackendType::D3D11:
        case wgpu::BackendType::OpenGL:
        case wgpu::BackendType::OpenGLES:
            return 10;
        case wgpu::BackendType::Undefined:
            return 100;
    }
}

void inspectAdapter(WGPUAdapter adapter)
{
    std::vector<WGPUFeatureName> features;

    // Call the function a first time with a null return address, just to get
    // the entry count.
    size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);

    // Allocate memory (could be a new, or a malloc() if this were a C program)
    features.resize(featureCount);

    // Call the function a second time, with a non-null return address
    wgpuAdapterEnumerateFeatures(adapter, features.data());

    std::cout << "Adapter features:" << std::endl;
    for (auto f : features)
    {
        std::cout << " - " << f << std::endl;
    }

    WGPUSupportedLimits limits = {};
    limits.nextInChain = nullptr;
    bool success = wgpuAdapterGetLimits(adapter, &limits);
    if (success) {
        std::cout << "Adapter limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;
        std::cout << " - maxBindGroups: " << limits.limits.maxBindGroups << std::endl;
        std::cout << " - maxDynamicUniformBuffersPerPipelineLayout: " << limits.limits.maxDynamicUniformBuffersPerPipelineLayout << std::endl;
        std::cout << " - maxDynamicStorageBuffersPerPipelineLayout: " << limits.limits.maxDynamicStorageBuffersPerPipelineLayout << std::endl;
        std::cout << " - maxSampledTexturesPerShaderStage: " << limits.limits.maxSampledTexturesPerShaderStage << std::endl;
        std::cout << " - maxSamplersPerShaderStage: " << limits.limits.maxSamplersPerShaderStage << std::endl;
        std::cout << " - maxStorageBuffersPerShaderStage: " << limits.limits.maxStorageBuffersPerShaderStage << std::endl;
        std::cout << " - maxStorageTexturesPerShaderStage: " << limits.limits.maxStorageTexturesPerShaderStage << std::endl;
        std::cout << " - maxUniformBuffersPerShaderStage: " << limits.limits.maxUniformBuffersPerShaderStage << std::endl;
        std::cout << " - maxUniformBufferBindingSize: " << limits.limits.maxUniformBufferBindingSize << std::endl;
        std::cout << " - maxStorageBufferBindingSize: " << limits.limits.maxStorageBufferBindingSize << std::endl;
        std::cout << " - minUniformBufferOffsetAlignment: " << limits.limits.minUniformBufferOffsetAlignment << std::endl;
        std::cout << " - minStorageBufferOffsetAlignment: " << limits.limits.minStorageBufferOffsetAlignment << std::endl;
        std::cout << " - maxVertexBuffers: " << limits.limits.maxVertexBuffers << std::endl;
        std::cout << " - maxVertexAttributes: " << limits.limits.maxVertexAttributes << std::endl;
        std::cout << " - maxVertexBufferArrayStride: " << limits.limits.maxVertexBufferArrayStride << std::endl;
        std::cout << " - maxInterStageShaderComponents: " << limits.limits.maxInterStageShaderComponents << std::endl;
        std::cout << " - maxComputeWorkgroupStorageSize: " << limits.limits.maxComputeWorkgroupStorageSize << std::endl;
        std::cout << " - maxComputeInvocationsPerWorkgroup: " << limits.limits.maxComputeInvocationsPerWorkgroup << std::endl;
        std::cout << " - maxComputeWorkgroupSizeX: " << limits.limits.maxComputeWorkgroupSizeX << std::endl;
        std::cout << " - maxComputeWorkgroupSizeY: " << limits.limits.maxComputeWorkgroupSizeY << std::endl;
        std::cout << " - maxComputeWorkgroupSizeZ: " << limits.limits.maxComputeWorkgroupSizeZ << std::endl;
        std::cout << " - maxComputeWorkgroupsPerDimension: " << limits.limits.maxComputeWorkgroupsPerDimension << std::endl;
    }

    WGPUAdapterProperties properties = {};
    properties.nextInChain = nullptr;
    wgpuAdapterGetProperties(adapter, &properties);
    std::cout << "Adapter properties:" << std::endl;
    std::cout << " - vendorID: " << properties.vendorID << std::endl;
    std::cout << " - deviceID: " << properties.deviceID << std::endl;
    std::cout << " - name: " << properties.name << std::endl;
    if (properties.driverDescription) {
        std::cout << " - driverDescription: " << properties.driverDescription << std::endl;
    }
    std::cout << " - adapterType: " << properties.adapterType << std::endl;
    std::cout << " - backendType: " << properties.backendType << std::endl;
}



// We also add an inspect device function:
void inspectDevice(WGPUDevice device) {
    std::vector<WGPUFeatureName> features;
    size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
    features.resize(featureCount);
    wgpuDeviceEnumerateFeatures(device, features.data());

    std::cout << "Device features:" << std::endl;
    for (auto f : features) {
        std::cout << " - " << f << std::endl;
    }

    WGPUSupportedLimits limits = {};
    limits.nextInChain = nullptr;
    bool success = wgpuDeviceGetLimits(device, &limits);
    if (success) {
        std::cout << "Device limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;
        std::cout << " - maxBindGroups: " << limits.limits.maxBindGroups << std::endl;
        std::cout << " - maxDynamicUniformBuffersPerPipelineLayout: " << limits.limits.maxDynamicUniformBuffersPerPipelineLayout << std::endl;
        std::cout << " - maxDynamicStorageBuffersPerPipelineLayout: " << limits.limits.maxDynamicStorageBuffersPerPipelineLayout << std::endl;
        std::cout << " - maxSampledTexturesPerShaderStage: " << limits.limits.maxSampledTexturesPerShaderStage << std::endl;
        std::cout << " - maxSamplersPerShaderStage: " << limits.limits.maxSamplersPerShaderStage << std::endl;
        std::cout << " - maxStorageBuffersPerShaderStage: " << limits.limits.maxStorageBuffersPerShaderStage << std::endl;
        std::cout << " - maxStorageTexturesPerShaderStage: " << limits.limits.maxStorageTexturesPerShaderStage << std::endl;
        std::cout << " - maxUniformBuffersPerShaderStage: " << limits.limits.maxUniformBuffersPerShaderStage << std::endl;
        std::cout << " - maxUniformBufferBindingSize: " << limits.limits.maxUniformBufferBindingSize << std::endl;
        std::cout << " - maxStorageBufferBindingSize: " << limits.limits.maxStorageBufferBindingSize << std::endl;
        std::cout << " - minUniformBufferOffsetAlignment: " << limits.limits.minUniformBufferOffsetAlignment << std::endl;
        std::cout << " - minStorageBufferOffsetAlignment: " << limits.limits.minStorageBufferOffsetAlignment << std::endl;
        std::cout << " - maxVertexBuffers: " << limits.limits.maxVertexBuffers << std::endl;
        std::cout << " - maxVertexAttributes: " << limits.limits.maxVertexAttributes << std::endl;
        std::cout << " - maxVertexBufferArrayStride: " << limits.limits.maxVertexBufferArrayStride << std::endl;
        std::cout << " - maxInterStageShaderComponents: " << limits.limits.maxInterStageShaderComponents << std::endl;
        std::cout << " - maxComputeWorkgroupStorageSize: " << limits.limits.maxComputeWorkgroupStorageSize << std::endl;
        std::cout << " - maxComputeInvocationsPerWorkgroup: " << limits.limits.maxComputeInvocationsPerWorkgroup << std::endl;
        std::cout << " - maxComputeWorkgroupSizeX: " << limits.limits.maxComputeWorkgroupSizeX << std::endl;
        std::cout << " - maxComputeWorkgroupSizeY: " << limits.limits.maxComputeWorkgroupSizeY << std::endl;
        std::cout << " - maxComputeWorkgroupSizeZ: " << limits.limits.maxComputeWorkgroupSizeZ << std::endl;
        std::cout << " - maxComputeWorkgroupsPerDimension: " << limits.limits.maxComputeWorkgroupsPerDimension << std::endl;
    }
}

namespace Walnut
{
    void GlfwWebGPURenderingBackend::Init(GLFWwindow *windowHandle)
    {
        m_windowHandle = windowHandle;

        m_wgpuNativeInstance = std::make_unique<dawn::native::Instance>();

        DawnProcTable procs = dawn::native::GetProcs();
        dawnProcSetProcs(&procs);
        m_wgpuInstance = wgpu::Instance(m_wgpuNativeInstance->Get());

        // 3. We can check whether there is actually an instance created
        if (!m_wgpuInstance) {
            std::cerr << "Could not initialize WebGPU!" << std::endl;
            return;
        }

        // 4. Display the object (WGPUInstance is a simple pointer, it may be
        // copied around without worrying about its size).
        std::cout << "WGPU instance: " << m_wgpuInstance.Get() << std::endl;

        std::cout << "Requesting adapter..." << std::endl;

        m_wgpuNativeInstance->DiscoverDefaultAdapters();
        auto adapters = m_wgpuNativeInstance->GetAdapters();

        // Sort adapters by adapterType, 
        std::sort(adapters.begin(), adapters.end(), [](const dawn::native::Adapter& a, const dawn::native::Adapter& b){
            wgpu::AdapterProperties pa, pb;
            a.GetProperties(&pa);
            b.GetProperties(&pb);
            
            if (pa.adapterType != pb.adapterType) {
                // Put GPU adapter (D3D, Vulkan, Metal) at front and CPU adapter at back.
                return pa.adapterType < pb.adapterType;
            }

            return GetBackendPriority(pa.backendType) < GetBackendPriority(pb.backendType);
        });
        // Simply pick the first adapter in the sorted list.
        dawn::native::Adapter backendAdapter = adapters[0];

        std::cout << "Got adapter: " << backendAdapter.Get() << std::endl;

        inspectAdapter(backendAdapter.Get());



        std::cout << "Requesting device..." << std::endl;

        

        WGPUDeviceDescriptor deviceDesc = {};
        deviceDesc.nextInChain = nullptr;
        deviceDesc.label = "My Device"; // anything works here, that's your call
        deviceDesc.requiredFeaturesCount = 0; // we do not require any specific feature
        deviceDesc.requiredLimits = nullptr; // we do not require any specific limit
        deviceDesc.defaultQueue.nextInChain = nullptr;
        deviceDesc.defaultQueue.label = "The default queue";
        m_wgpuDevice = wgpu::Device::Acquire(backendAdapter.CreateDevice(&deviceDesc));
        //m_wgpuDevice = emscripten_webgpu_get_device();

        std::cout << "Got device: " << m_wgpuDevice.Get() << std::endl;

        auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) {
            std::cout << "Uncaptured device error: type " << type;
            if (message) std::cout << " (" << message << ")";
            std::cout << std::endl;
        };
        wgpuDeviceSetUncapturedErrorCallback(m_wgpuDevice.Get(), onDeviceError, nullptr /* pUserData */);


        inspectDevice(m_wgpuDevice.Get());

        

    }
    void GlfwWebGPURenderingBackend::SetupGraphicsAPI()
    {
    }

    void GlfwWebGPURenderingBackend::SetupWindow(int width, int height)
    {
        WGPUSurface surface = glfwGetWGPUSurface(m_wgpuInstance.Get(), m_windowHandle);
        std::cout << "Got surface: " << surface << std::endl;

        ImGui_ImplWGPU_InvalidateDeviceObjects();

        wgpu::SwapChainDescriptor swapChainDesc = {};
        swapChainDesc.nextInChain = nullptr;
        swapChainDesc.width = width;
        swapChainDesc.height = height;
        //WGPUTextureFormat swapChainFormat = wgpuSurfaceGetPreferredFormat(surface, adapter); // because Dawn still do not have a implementation for wgpuSurfaceGetPreferredFormat
        WGPUTextureFormat swapChainFormat = m_wgpuPreferredFormat;
        swapChainDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
        m_swapChain = m_wgpuDevice.CreateSwapChain(surface, &swapChainDesc);
        std::cout << "Swapchain: " << m_swapChain.Get() << std::endl;

        ImGui_ImplWGPU_CreateDeviceObjects();
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
        ImGui_ImplWGPU_Init(m_wgpuDevice.Get(), 3, m_wgpuPreferredFormat, WGPUTextureFormat_Undefined);
    }

    void GlfwWebGPURenderingBackend::StartImGuiFrame()
    {
        // Start the Dear ImGui frame
		ImGui_ImplWGPU_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    void GlfwWebGPURenderingBackend::UploadFonts()
    {
    }

    void GlfwWebGPURenderingBackend::FrameRender(ImDrawData *draw_data)
    {
        wgpu::TextureView nextTexture = m_swapChain.GetCurrentTextureView();
        if (!nextTexture) {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            return;
        }
        std::cout << "nextTexture: " << nextTexture.Get() << std::endl;

        wgpu::CommandEncoderDescriptor enc_desc = {};
        m_commandEncoder = m_wgpuDevice.CreateCommandEncoder(&enc_desc);

        wgpu::RenderPassColorAttachment color_attachments = {};
        color_attachments.loadOp = wgpu::LoadOp::Clear;
        color_attachments.storeOp = wgpu::StoreOp::Store;
        static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        color_attachments.clearValue = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        color_attachments.view = nextTexture;

        wgpu::RenderPassDescriptor render_pass_desc = {};
        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &color_attachments;
        render_pass_desc.depthStencilAttachment = nullptr;
        wgpu::RenderPassEncoder pass = m_commandEncoder.BeginRenderPass(&render_pass_desc);

        ImGui_ImplWGPU_RenderDrawData(draw_data, pass.Get());
        pass.Draw(3);
        pass.End();
    }

    void GlfwWebGPURenderingBackend::FramePresent()
    {
        wgpu::CommandBufferDescriptor cmd_buffer_desc = {};
        wgpu::CommandBuffer cmd_buffer = m_commandEncoder.Finish(&cmd_buffer_desc);
        wgpu::Queue queue = m_wgpuDevice.GetQueue();
        queue.Submit(1, &cmd_buffer);
        m_swapChain.Present();
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