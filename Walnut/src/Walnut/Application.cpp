#include "Application.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "GraphicsAPI/VulkanGraphics.h"

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5_webgpu.h>
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

static void SetupGraphicsAPI(const char** extensions, uint32_t extensions_count)
{
#ifdef __EMSCRIPTEN__
    	SetupWebGPU(extensions, extensions_count);
#else
		VulkanGraphicsAPI::SetupVulkan(extensions, extensions_count);
#endif	
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupGraphicsAPIWindow(int width, int height)
{
	VulkanGraphicsAPI::SetupVulkanWindow(width, height);
}

static void CleanupGraphicsAPI()
{
	VulkanGraphicsAPI::CleanupVulkan();
}

static void CleanupGraphicsAPIWindow()
{
	VulkanGraphicsAPI::CleanupVulkanWindow();
}

static void FrameRender(ImDrawData* draw_data)
{
	VulkanGraphicsAPI::FrameRender(draw_data);
}

static void FramePresent()
{
	VulkanGraphicsAPI::FramePresent();
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

namespace Walnut {

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		s_Instance = this;

		Init();
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

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_WindowHandle = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Name.c_str(), NULL, NULL);

		// Setup Vulkan
		if (!glfwVulkanSupported())
		{
			std::cerr << "GLFW: Vulkan not supported!\n";
			return;
		}
		uint32_t extensions_count = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
		SetupGraphicsAPI(extensions, extensions_count);

		// Create Window Surface
		VulkanGraphicsAPI::AddWindowHandle(m_WindowHandle);

		// Create Framebuffers
		int w, h;
		glfwGetFramebufferSize(m_WindowHandle, &w, &h);
		SetupGraphicsAPIWindow(w, h);

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

		// Setup Platform/Renderer backends
		VulkanGraphicsAPI::ConfigureRendererBackend(m_WindowHandle);

		// Load default font
		ImFontConfig fontConfig;
		fontConfig.FontDataOwnedByAtlas = false;
		ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
		io.FontDefault = robotoFont;

		// Upload Fonts
		{
			VulkanGraphicsAPI::UploadFonts();
		}
	}

	void Application::Shutdown()
	{
		for (auto& layer : m_LayerStack)
			layer->OnDetach();

		m_LayerStack.clear();

		VulkanGraphicsAPI::GraphicsDeviceWaitIdle();

		VulkanGraphicsAPI::FreeGraphicsResources();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		CleanupGraphicsAPIWindow();
		CleanupGraphicsAPI();

		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();

		g_ApplicationRunning = false;
	}

	void Application::Run()
	{
		m_Running = true;
		ImGuiIO& io = ImGui::GetIO();

		// Main loop
		while (!glfwWindowShouldClose(m_WindowHandle) && m_Running)
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
			if (VulkanGraphicsAPI::NeedSwapChainRebuild())
			{
				int width, height;
				glfwGetFramebufferSize(m_WindowHandle, &width, &height);
				if (width > 0 && height > 0)
				{
					VulkanGraphicsAPI::ResizeVulkanWindow(width, height);
					VulkanGraphicsAPI::SetSwapChainRebuildStatus(false);
				}
			}

			// Start the Dear ImGui frame
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			{
				static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

				// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
				// because it would be confusing to have two docking targets within each others.
				ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
				if (m_MenubarCallback)
					window_flags |= ImGuiWindowFlags_MenuBar;

				const ImGuiViewport* viewport = ImGui::GetMainViewport();
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
				ImGui::Begin("DockSpace Demo", nullptr, window_flags);
				ImGui::PopStyleVar();

				ImGui::PopStyleVar(2);

				// Submit the DockSpace
				ImGuiIO& io = ImGui::GetIO();
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

				for (auto& layer : m_LayerStack)
					layer->OnUIRender();

				ImGui::End();
			}

			// Rendering
			ImGui::Render();
			ImDrawData* main_draw_data = ImGui::GetDrawData();
			const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
			VulkanGraphicsAPI::SetClearColor(ImVec4(0.45f, 0.55f, 0.60f, 1.00f));
			if (!main_is_minimized)
				FrameRender(main_draw_data);

			// Update and Render additional Platform Windows
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			// Present Main Platform Window
			if (!main_is_minimized)
				FramePresent();

			float time = GetTime();
			m_FrameTime = time - m_LastFrameTime;
			m_TimeStep = glm::min<float>(m_FrameTime, 0.0333f);
			m_LastFrameTime = time;
		}

	}

	void Application::Close()
	{
		m_Running = false;
	}

	float Application::GetTime()
	{
		return (float)glfwGetTime();
	}

	VkInstance Application::GetInstance1()
	{
		return nullptr;//g_Instance;
	}

	VkPhysicalDevice Application::GetPhysicalDevice1()
	{
		return nullptr;//g_PhysicalDevice;
	}

	VkDevice Application::GetDevice1()
	{
		return nullptr;//g_Device;
	}

	VkCommandBuffer Application::GetGraphicsCommandBuffer(bool begin)
	{
		return VulkanGraphicsAPI::GetCommandBuffer(begin);
	}

	void Application::FlushGraphicsCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VulkanGraphicsAPI::FlushCommandBuffer(commandBuffer);
	}

	void Application::SubmitGraphicsResourceFree() // std::function<void()>&& func
	{
		VulkanGraphicsAPI::SubmitResourceFree(); //std::move(func)
	}

}
