#include "Application.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include <iostream>
#include <thread>
#include <imgui.h>

#include <imgui_impl_sdl3.h>
#include <SDL3/SDL.h>

#include "RenderingBackend.h"
#include "Image.h"

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

	void Application::OnWindowResize(WalnutWindowHandleType *win)
    {
		int w, h;
		SDL_GetWindowSize(win, &w, &h);
		m_FramebufferWidth = w > 0 ? static_cast<uint32_t>(w) : 0;
		m_FramebufferHeight = h > 0 ? static_cast<uint32_t>(h) : 0;
		if (w <= 0 || h <= 0) {
			return; // minimised: a zero-extent swapchain or image is invalid, keep the old ones until restored
		}
		m_RenderingBackend->SetupWindow(w, h);
		std::cout << "Resized window to: x=" << w << ", y=" << h << std::endl;
		if (m_Specification.UseImGui || m_UseExternalMainImage) {
			return; // when using ImGui or an external main image handle, Walnut does not manage m_ImageToRender
		}
		m_ImageToRender = std::make_unique<Image>(w, h, ImageFormat::RGBA);
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
			sdlWindowType = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
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
			std::cerr << "Could not create SDL Window: " << SDL_GetError() << "\n";
			assert(false);
			return;
		}

		m_RenderingBackend->Init(windowHandle, m_Specification.EnableGraphicsValidation);

		OnWindowResize(windowHandle);
		if (m_Specification.UseImGui)
		{
			InitImGui();
			// Setup Platform/Renderer backends to work with ImGui
			m_RenderingBackend->CreateImGuiPipeline();
		}
		else
		{
			// when not using ImGui, we need to create our main image pipeline here,
			m_RenderingBackend->CreateMainImagePipeline();
		}
	}

	void Application::Shutdown()
	{
		LayerStackShutdown();
		m_ImageToRender.reset();
		m_RenderingBackend->Shutdown();

		if (m_Specification.UseImGui) {
			m_RenderingBackend->DestroyImGuiPipeline();
			ImGui::DestroyContext();
		} else {
			m_RenderingBackend->DestroyMainImagePipeline();
		}

		m_RenderingBackend->Cleanup();

		g_ApplicationRunning = false;
	}

	float Application::QueryUIScale() const
	{
		// The display scale is pixel density * content scale. Where window coordinates are logical points (macOS,
		// Wayland) ImGui applies the pixel density through io.DisplayFramebufferScale, so only the content scale is left.
		WalnutWindowHandleType* window = m_RenderingBackend->GetWindowHandle();
		const float displayScale = SDL_GetWindowDisplayScale(window);
		const float pixelDensity = SDL_GetWindowPixelDensity(window);
		if (displayScale <= 0.0f || pixelDensity <= 0.0f)
			return 1.0f;
		return displayScale / pixelDensity;
	}

	void Application::MainLoop()
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
			if (m_Specification.UseImGui) {
				ImGui_ImplSDL3_ProcessEvent(&event);
			}

			if (event.type == SDL_EVENT_QUIT) {
                // The user requested to quit the application (e.g., closed the main window)
                m_Running = false;
            }

			if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				OnWindowResize(m_RenderingBackend->GetWindowHandle());
			}

			if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
				OnUIScaleChanged(); // e.g. the window moved to a monitor with a different scale
			}
        }

		LayerStackOnUpdate();

		// Resize swap chain?
		if (m_RenderingBackend->NeedToResizeWindow())
		{
			int width, height;
			//glfwGetFramebufferSize(m_RenderingBackend->GetWindowHandle(), &width, &height);
            SDL_GetWindowSize(m_RenderingBackend->GetWindowHandle(), &width, &height);
			m_FramebufferWidth = width > 0 ? static_cast<uint32_t>(width) : 0;
			m_FramebufferHeight = height > 0 ? static_cast<uint32_t>(height) : 0;
			if (width > 0 && height > 0)
				m_RenderingBackend->ResizeWindow(width, height);
		}

		bool main_is_minimized = false;
		if (m_Specification.UseImGui) {
			m_RenderingBackend->StartImGuiFrame();
			ImGui::NewFrame();
			SetupImGuiForOneIteration();
			ImGui::EndFrame();
			ImGui::Render();

			ImDrawData* main_draw_data = ImGui::GetDrawData();
			main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
		} else {
			const Uint32 flags = SDL_GetWindowFlags(m_RenderingBackend->GetWindowHandle());
			main_is_minimized = (flags & SDL_WINDOW_MINIMIZED) != 0;
		}

		if (!main_is_minimized) {
			m_RenderingBackend->FrameBegin();
			if (m_Specification.UseImGui) {
				ImDrawData* main_draw_data = ImGui::GetDrawData();
				m_RenderingBackend->FrameRenderImGui(main_draw_data);
			} else if (m_UseExternalMainImage) {
				m_RenderingBackend->FrameRender(m_ExternalMainImageHandle);
			} else {
				m_RenderingBackend->FrameRender(m_ImageToRender);
			}
			m_RenderingBackend->FrameEnd();
			CaptureFrameIfRequested();
		}

		if (m_Specification.UseImGui) {
			// Update and Render additional Platform Windows
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				auto* backupPtr = SDL_GL_GetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				SDL_GL_MakeCurrent(m_RenderingBackend->GetWindowHandle(), backupPtr);
			}
		}

		// Present Main Platform Window
		if (!main_is_minimized)
			m_RenderingBackend->FramePresent();

		const auto now = std::chrono::steady_clock::now();
		const float timeDiff = std::chrono::duration_cast<std::chrono::duration<float>>(now - m_LastFrameTimePoint).count();
		m_TimeStep = std::min<float>(timeDiff, 0.1f);
		m_LastFrameTimePoint = now;
	}

	void Application::Run()
	{
		m_Running = true;

		// Main loop
		while (m_Running)
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
