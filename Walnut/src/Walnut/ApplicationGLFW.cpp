#include "Application.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include <iostream>
#include <thread>
#include <imgui.h>
#include <GLFW/glfw3.h>

#include <Walnut/GLM/GLM.h>
#include <imgui_impl_glfw.h>

#include "RenderingBackend.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/emscripten.h>
#endif

// Emedded font
#include "ImGui/Roboto-Regular.embed"

Walnut_API bool g_ApplicationRunning = true;

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
		, m_SleepAmount(std::chrono::milliseconds(10))
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

	void Application::OnWindowResize(GLFWwindow *win, int width, int height)
    {
		std::cout << "Resized window to: x=" << width << ", y=" << height << std::endl;
		// Create Framebuffers
		{
			int w, h;
			glfwGetFramebufferSize(win, &w, &h);
			m_RenderingBackend->SetupWindow(w, h);
		}
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
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
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

		glfwSetWindowUserPointer(windowHandle, this);
		glfwSetWindowSizeCallback(windowHandle, [](GLFWwindow* win, int width, int height) {
			auto app = static_cast<Application*>(glfwGetWindowUserPointer(win));
			assert(app);
			app->OnWindowResize(win, width, height);
		});

		// Create Framebuffers
		{
			int w, h;
			glfwGetFramebufferSize(windowHandle, &w, &h);
			m_RenderingBackend->SetupWindow(w, h);
		}
		
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
		m_RenderingBackend->ConfigureImGui();

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
		LayerStackShutdown();

		m_RenderingBackend->Shutdown();

		ImGui::DestroyContext();

		m_RenderingBackend->Cleanup();

		glfwDestroyWindow(m_RenderingBackend->GetWindowHandle());
		glfwTerminate();

		g_ApplicationRunning = false;
	}

	void Application::MainLoop()
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		LayerStackOnUpdate();

		// Resize swap chain?
		if (m_RenderingBackend->NeedToResizeWindow())
		{
			int width, height;
			glfwGetFramebufferSize(m_RenderingBackend->GetWindowHandle(), &width, &height);
			if (width > 0 && height > 0)
				m_RenderingBackend->ResizeWindow(width, height);
		}

		m_RenderingBackend->StartImGuiFrame();
		SetupImGuiForOneIteration();
		ImGui::EndFrame();

		// Rendering
		ImGui::Render();
		ImDrawData* main_draw_data = ImGui::GetDrawData();
		const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
		if (!main_is_minimized)
			m_RenderingBackend->FrameRender(main_draw_data);

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
		if (!main_is_minimized)
			m_RenderingBackend->FramePresent();

		const auto now = std::chrono::steady_clock::now();
		const float timeDiff = std::chrono::duration_cast<std::chrono::duration<float>>(now - m_LastFrameTimePoint).count();
		m_TimeStep = glm::min<float>(timeDiff, 0.1f);
		m_LastFrameTimePoint = now;
	}

	void Application::Run()
	{
		m_Running = true;

#ifdef __EMSCRIPTEN__
    	emscripten_set_main_loop_arg(&EmscriptenMainLoop, this, 0, true);
#else
		// Main loop
		while (!glfwWindowShouldClose(m_RenderingBackend->GetWindowHandle()) && m_Running)
		{
			MainLoop();
			std::this_thread::sleep_for(m_SleepAmount);
		}
#endif

	}

	float Application::GetTime()
	{
		return (float)glfwGetTime();
	}

}
