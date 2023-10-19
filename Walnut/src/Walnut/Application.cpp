#include "Application.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include <iostream>

#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../../../vendor/glfw3webgpu/glfw3webgpu.h"

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "RenderingBackend.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/emscripten.h>
#endif

// Emedded font
#include "ImGui/Roboto-Regular.embed"

extern bool g_ApplicationRunning;

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static Walnut::Application* s_Instance = nullptr;

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void EmscriptenMainLoop(void* arg)
{
  static_cast<Walnut::Application*>(arg)->MainLoop();
}

namespace Walnut {

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
		, m_RenderingBackend(std::move(RenderingBackend::Create()))
	{
		s_Instance = this;

		//Init();
	}

	Application::~Application()
	{
		Shutdown();

		s_Instance = nullptr;
	}

	Application& Application::Get()
	{
		return *s_Instance;
	}

	void Application::Init()
	{
		// Setup GLFW window
		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
		{
			std::cerr << "Could not initalize GLFW!\n";
			return;
		}

		if (RenderingBackend::GetBackend() == RenderingBackend::BACKEND::OpenGL)
		{
#ifdef __EMSCRIPTEN__
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#else
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // use GLFW_OPENGL_ANY_PROFILE for X11 sessions
#endif

			// glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
			// glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			// glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		}
		else if (RenderingBackend::GetBackend() == RenderingBackend::BACKEND::Vulkan)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		else if (RenderingBackend::GetBackend() == RenderingBackend::BACKEND::WebGPU)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		else
		{
			assert(false);
		}

		auto* windowHandle = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Name.c_str(), NULL, NULL);
		if (!windowHandle)
		{
			std::cerr << "Could not create GLFW Window!\n";
			assert(false);
			return;
		}

		m_RenderingBackend->Init(windowHandle);
		m_RenderingBackend->SetupGraphicsAPI();

		// Create Framebuffers
		int w, h;
		glfwGetFramebufferSize(windowHandle, &w, &h);
		m_RenderingBackend->SetupWindow(w, h);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends to work with ImGui
		//m_RenderingBackend->ConfigureImGui();

		// Load default font
		ImFontConfig fontConfig;
		fontConfig.FontDataOwnedByAtlas = false;
		ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
		io.FontDefault = robotoFont;

		// Upload Fonts
		m_RenderingBackend->UploadFonts();
	}

	void Application::Shutdown()
	{
		for (auto& layer : m_LayerStack)
			layer->OnDetach();

		m_LayerStack.clear();

		m_RenderingBackend->Shutdown();

		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		m_RenderingBackend->Cleanup();

		glfwDestroyWindow(m_RenderingBackend->GetWindowHandle());
		glfwTerminate();

		g_ApplicationRunning = false;
	}

	void Application::SetupImGui()
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		if (m_MenubarCallback)
			window_flags |= ImGuiWindowFlags_MenuBar;

		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		static bool dockspaceOpen = true;
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO &io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (m_MenubarCallback)
		{
			if (ImGui::BeginMenuBar())
			{
				m_MenubarCallback();
				ImGui::EndMenuBar();
			}
		}

		if (m_UIRenderingCallback)
			m_UIRenderingCallback();
		else
			std::cout << "Error - applicationUIRenderingCallback is not set!" << std::endl;

		ImGui::End();
	}

	void Application::MainLoop()
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		for (auto& layer : m_LayerStack)
			layer->OnUpdate(m_TimeStep);

		// Resize swap chain?
		if (m_RenderingBackend->NeedToResizeWindow())
		{
			int width, height;
			glfwGetFramebufferSize(m_RenderingBackend->GetWindowHandle(), &width, &height);
			if (width > 0 && height > 0)
				m_RenderingBackend->ResizeWindow(width, height);
		}

		// m_RenderingBackend->StartImGuiFrame();
		// SetupImGui();
		// ImGui::EndFrame();

		// // Rendering
		// ImGui::Render();
		// ImDrawData* main_draw_data = ImGui::GetDrawData();
		//const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
		//m_RenderingBackend->SetClearColor(ImVec4(0.45f, 0.55f, 0.60f, 1.00f));
		//if (!main_is_minimized)
		m_RenderingBackend->FrameRender(nullptr);

		// Update and Render additional Platform Windows
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			auto* backupPtr = glfwGetCurrentContext();  // save currentcontext and have to call glfwMakeContextCurrent later
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backupPtr); // if we do not do this there will be a bug in opengl when docking
		}

		// Present Main Platform Window
		//if (!main_is_minimized)
		m_RenderingBackend->FramePresent();

		float time = GetTime();
		m_FrameTime = time - m_LastFrameTime;
		m_TimeStep = glm::min<float>(m_FrameTime, 0.0333f);
		m_LastFrameTime = time;

	}

	wgpu::Instance m_instance = nullptr;
	GLFWwindow* m_window = nullptr;
	wgpu::Surface m_surface = nullptr;
	wgpu::Device m_device = nullptr;
	wgpu::TextureFormat m_swapChainFormat = wgpu::TextureFormat::Undefined;
	wgpu::Queue m_queue = nullptr;
	wgpu::SwapChain m_swapChain = nullptr;
	wgpu::TextureFormat m_depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
	wgpu::Texture m_depthTexture = nullptr;
	wgpu::TextureView m_depthTextureView = nullptr;

	bool initWindowAndDevice() 
	{
		m_instance = createInstance(wgpu::InstanceDescriptor{});
		if (!m_instance) {
			std::cerr << "Could not initialize WebGPU!" << std::endl;
			return false;
		}

		if (!glfwInit()) {
			std::cerr << "Could not initialize GLFW!" << std::endl;
			return false;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		m_window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);
		if (!m_window) {
			std::cerr << "Could not open window!" << std::endl;
			return false;
		}

		std::cout << "Requesting adapter..." << std::endl;
		m_surface = glfwGetWGPUSurface(m_instance, m_window);
		wgpu::RequestAdapterOptions adapterOpts{};
		adapterOpts.compatibleSurface = m_surface;
		wgpu::Adapter adapter = m_instance.requestAdapter(adapterOpts);
		std::cout << "Got adapter: " << adapter << std::endl;

		wgpu::SupportedLimits supportedLimits;
		adapter.getLimits(&supportedLimits);

		std::cout << "Requesting device..." << std::endl;
		wgpu::RequiredLimits requiredLimits = wgpu::Default;
		requiredLimits.limits.maxVertexAttributes = 4;
		requiredLimits.limits.maxVertexBuffers = 1;

		struct VertexAttributes {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 color;
			glm::vec2 uv;
		};
		requiredLimits.limits.maxBufferSize = 150000 * sizeof(VertexAttributes);
		requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
		requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
		requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
		requiredLimits.limits.maxInterStageShaderComponents = 8;
		requiredLimits.limits.maxBindGroups = 2;
		//                                    ^ This was a 1
		requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
		requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
		// Allow textures up to 2K
		requiredLimits.limits.maxTextureDimension1D = 2048;
		requiredLimits.limits.maxTextureDimension2D = 2048;
		requiredLimits.limits.maxTextureArrayLayers = 1;
		requiredLimits.limits.maxSampledTexturesPerShaderStage = 1;
		requiredLimits.limits.maxSamplersPerShaderStage = 1;

		wgpu::DeviceDescriptor deviceDesc;
		deviceDesc.label = "My Device";
		deviceDesc.requiredFeaturesCount = 0;
		deviceDesc.requiredLimits = &requiredLimits;
		deviceDesc.defaultQueue.label = "The default queue";
		m_device = adapter.requestDevice(deviceDesc);
		adapter.release();
		std::cout << "Got device: " << m_device << std::endl;

		// Add an error callback for more debug info
		std::unique_ptr<wgpu::ErrorCallback> m_errorCallbackHandle;
		m_errorCallbackHandle = m_device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
			std::cout << "Device error: type " << type;
			if (message) std::cout << " (message: " << message << ")";
			std::cout << std::endl;
		});

		m_queue = m_device.getQueue();

#ifdef WEBGPU_BACKEND_WGPU
		m_swapChainFormat = m_surface.getPreferredFormat(adapter);
#else
		m_swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif

		return m_device != nullptr;
	}

	bool initSwapChain() {
		// Get the current size of the window's framebuffer:
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);

		std::cout << "Creating swapchain..." << std::endl;
		wgpu::SwapChainDescriptor swapChainDesc;
		swapChainDesc.width = static_cast<uint32_t>(width);
		swapChainDesc.height = static_cast<uint32_t>(height);
		swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
		swapChainDesc.format = m_swapChainFormat;
		swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
		m_swapChain = m_device.createSwapChain(m_surface, swapChainDesc);
		std::cout << "Swapchain: " << m_swapChain << std::endl;
		return m_swapChain != nullptr;
	}

	bool initDepthBuffer() {
		// Get the current size of the window's framebuffer:
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);

		// Create the depth texture
		wgpu::TextureDescriptor depthTextureDesc;
		depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
		depthTextureDesc.format = m_depthTextureFormat;
		depthTextureDesc.mipLevelCount = 1;
		depthTextureDesc.sampleCount = 1;
		depthTextureDesc.size = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
		depthTextureDesc.viewFormatCount = 1;
		depthTextureDesc.viewFormats = (WGPUTextureFormat*)&m_depthTextureFormat;
		m_depthTexture = m_device.createTexture(depthTextureDesc);
		std::cout << "Depth texture: " << m_depthTexture << std::endl;

		// Create the view of the depth texture manipulated by the rasterizer
		wgpu::TextureViewDescriptor depthTextureViewDesc;
		depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
		depthTextureViewDesc.baseArrayLayer = 0;
		depthTextureViewDesc.arrayLayerCount = 1;
		depthTextureViewDesc.baseMipLevel = 0;
		depthTextureViewDesc.mipLevelCount = 1;
		depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
		depthTextureViewDesc.format = m_depthTextureFormat;
		m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);
		std::cout << "Depth texture view: " << m_depthTextureView << std::endl;

		return m_depthTextureView != nullptr;
	}

	void updateGui(wgpu::RenderPassEncoder renderPass) 
	{
		// Start the Dear ImGui frame
		ImGui_ImplWGPU_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Build our UI
		{
			static float f = 0.0f;
			static int counter = 0;
			static bool show_demo_window = true;
			static bool show_another_window = false;
			static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

			ImGui::Begin("Hello, world!");                                // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");                     // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);            // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);                  // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color);       // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                                  // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		// Draw the UI
		ImGui::EndFrame();
		// Convert the UI defined above into low-level drawing commands
		ImGui::Render();
		// Execute the low-level drawing commands on the WebGPU backend
		ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
	}

	void debugMainNew()
	{
		initWindowAndDevice();
		initSwapChain();
		initDepthBuffer();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); 
		// (void)io;
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		// //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		// //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		// //io.ConfigViewportsNoAutoMerge = true;
		// //io.ConfigViewportsNoTaskBarIcon = true;

		ImGui_ImplGlfw_InitForOther(m_window, true);
        ImGui_ImplWGPU_Init(m_device, 3, m_swapChainFormat, m_depthTextureFormat);
		//ImGui_ImplWGPU_CreateDeviceObjects();

		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();

			wgpu::TextureView nextTexture = m_swapChain.getCurrentTextureView();
			if (!nextTexture) {
				std::cerr << "Cannot acquire next swap chain texture" << std::endl;
				return;
			}

			wgpu::CommandEncoderDescriptor commandEncoderDesc;
			commandEncoderDesc.label = "Command Encoder";
			wgpu::CommandEncoder encoder = m_device.createCommandEncoder(commandEncoderDesc);
			
			wgpu::RenderPassDescriptor renderPassDesc{};

			wgpu::RenderPassColorAttachment renderPassColorAttachment{};
			renderPassColorAttachment.view = nextTexture;
			renderPassColorAttachment.resolveTarget = nullptr;
			renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
			renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
			renderPassColorAttachment.clearValue = wgpu::Color{ 0.05, 0.05, 0.05, 1.0 };
			renderPassDesc.colorAttachmentCount = 1;
			renderPassDesc.colorAttachments = &renderPassColorAttachment;

			wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
			depthStencilAttachment.view = m_depthTextureView;
			depthStencilAttachment.depthClearValue = 1.0f;
			depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
			depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
			depthStencilAttachment.depthReadOnly = false;
			depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
			depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
			depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
#else
			depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
			depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
#endif
			depthStencilAttachment.stencilReadOnly = true;

			renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

			renderPassDesc.timestampWriteCount = 0;
			renderPassDesc.timestampWrites = nullptr;
			wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

			updateGui(renderPass);

			renderPass.end();
	
			nextTexture.release();

			wgpu::CommandBufferDescriptor cmdBufferDescriptor{};
			cmdBufferDescriptor.label = "Command buffer";
			wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
			m_queue.submit(command);

			m_swapChain.present();

#ifdef WEBGPU_BACKEND_DAWN
			// Check for pending error callbacks
			m_device.tick();
#endif
		}

		
	}

	void Application::Run()
	{
		m_Running = true;

// #ifdef __EMSCRIPTEN__
//     	emscripten_set_main_loop_arg(&EmscriptenMainLoop, this, 0, true);
// #else
// 		// Main loop
// 		while (!glfwWindowShouldClose(m_RenderingBackend->GetWindowHandle()) && m_Running)
// 			MainLoop();
// #endif

		debugMainNew();

	}

    void Application::SetMenubarCallback(const std::function<void()> &menubarCallback)
    {
		m_MenubarCallback = menubarCallback; 
    }

    void Application::SetUIRenderingCallback()
    {
		m_UIRenderingCallback = [layerStack = m_LayerStack](){
			for (auto& layer : layerStack)
				layer->OnUIRender();
		};
    }

    void Application::Close()
	{
		m_Running = false;
	}

	float Application::GetTime()
	{
		return (float)glfwGetTime();
	}

    GLFWwindow* Application::GetWindowHandle() const
    {
        return m_RenderingBackend->GetWindowHandle();
    }

}
