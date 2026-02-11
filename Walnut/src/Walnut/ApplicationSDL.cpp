#include "Application.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include <iostream>
#include <thread>
#include <imgui.h>

#include <Walnut/GLM/GLM.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL.h>

#include "RenderingBackend.h"

// Emedded font
#include "ImGui/Roboto-Regular.embed"

Walnut_API bool g_ApplicationRunning = true;

static Walnut::Application* s_Instance = nullptr;


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

	void Application::OnWindowResize(WalnutWindowHandleType *win, int width, int height)
    {
		std::cout << "Resized window to: x=" << width << ", y=" << height << std::endl;
		// Create Framebuffers
		// {
		// 	int w, h;
		// 	glfwGetFramebufferSize(win, &w, &h);
		// 	m_RenderingBackend->SetupWindow(w, h);
		// }
    }

	void Application::Init()
	{
		// Setup GLFW window
		//glfwSetErrorCallback(glfw_error_callback);
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			std::cerr << "Could not initalize GLFW!\n";
			assert(false);
			return;
		}

		uint32_t sdlWindowType;
		if (RenderingBackend::GetBackend() == RenderingBackend::BACKEND::OpenGL)
		{
			sdlWindowType = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#if defined(__EMSCRIPTEN__)
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#elif defined(__ANDROID__)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
		}
		else if (RenderingBackend::GetBackend() == RenderingBackend::BACKEND::Vulkan)
		{
			sdlWindowType = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY; // SDL_WINDOW_HIDDEN
		}
		else if (RenderingBackend::GetBackend() == RenderingBackend::BACKEND::WebGPU)
		{
			// glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			// glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			sdlWindowType = 0;
		}
		else
		{
			assert(false);
		}

        auto* windowHandle = SDL_CreateWindow(m_Specification.Name.c_str(), 
											m_Specification.Width, m_Specification.Height, 
											sdlWindowType);
		if (!windowHandle)
		{
			std::cerr << "Could not create GLFW Window!\n";
			assert(false);
			return;
		}

		m_RenderingBackend->Init(windowHandle);

		// glfwSetWindowUserPointer(windowHandle, this);
		// glfwSetWindowSizeCallback(windowHandle, [](GLFWwindow* win, int width, int height) {
		// 	auto app = static_cast<Application*>(glfwGetWindowUserPointer(win));
		// 	assert(app);
		// 	app->OnWindowResize(win, width, height);
		// });

		int windowWidth, windowHeight;
		SDL_GetWindowSize(windowHandle, &windowWidth, &windowHeight);
		m_RenderingBackend->SetupWindow(windowWidth, windowHeight);
		
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

		float scaleFactor = SDL_GetWindowDisplayScale(m_RenderingBackend->GetWindowHandle());
		std::cout << "#### SDL UI Scale: " << scaleFactor << std::endl;

		int widthInPixels, heightInPixels;
		SDL_GetWindowSizeInPixels(m_RenderingBackend->GetWindowHandle(), &widthInPixels, &heightInPixels);
		if (widthInPixels == 2 * windowWidth && heightInPixels == 2 * windowHeight) {
			// maybe a retina display
			scaleFactor /= 2.0f;
			std::cout << "#### SDL UI Scale (adjusted) : " << scaleFactor << std::endl;
		}

		// Setup SDL UI scaling for imgui
		style.ScaleAllSizes(scaleFactor);

		// Setup Platform/Renderer backends to work with ImGui
		m_RenderingBackend->ConfigureImGui();

		// Load default font
		ImFontConfig fontConfig;
		fontConfig.FontDataOwnedByAtlas = false;
		ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF(
								(void*)g_RobotoRegular, sizeof(g_RobotoRegular), 16.0f * scaleFactor, &fontConfig);
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

		// glfwDestroyWindow(m_RenderingBackend->GetWindowHandle());
		// glfwTerminate();

		g_ApplicationRunning = false;
	}

	void Application::MainLoop()
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		//glfwPollEvents();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            // if (event.type == SDL_QUIT)
            //     done = true;
            // if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            //     done = true;
        }

		LayerStackOnUpdate();

		// Resize swap chain?
		if (m_RenderingBackend->NeedToResizeWindow())
		{
			int width, height;
			//glfwGetFramebufferSize(m_RenderingBackend->GetWindowHandle(), &width, &height);
            SDL_GetWindowSize(m_RenderingBackend->GetWindowHandle(), &width, &height);
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
            auto* backupPtr = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(m_RenderingBackend->GetWindowHandle(), backupPtr);
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

		// Main loop
		while (m_Running) // !glfwWindowShouldClose(m_RenderingBackend->GetWindowHandle()) && 
		{
			MainLoop();
			std::this_thread::sleep_for(m_SleepAmount);
		}
	}

	float Application::GetTime()
	{
		return (float)SDL_GetTicks();
	}
}