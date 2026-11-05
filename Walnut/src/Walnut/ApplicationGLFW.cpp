#include "Application.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include <iostream>
#include <thread>
#include <imgui.h>
#include <GLFW/glfw3.h>

#include "RenderingBackend.h"
#include "Image.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/emscripten.h>
#endif

Walnut_API bool g_ApplicationRunning = true;

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static Walnut::Application* s_Instance = nullptr;

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

	void Application::OnWindowResize(WalnutWindowHandleType *win)
    {
		int w, h;
		glfwGetFramebufferSize(win, &w, &h);
		if (w == 0 || h == 0) {
			return; // minimised: a zero-extent swapchain or image is invalid, keep the old ones until restored
		}
		m_RenderingBackend->SetupWindow(w, h);

		if (m_Specification.UseImGui) {
			return; // when using ImGui, we don't need to resize our main image, as ImGui will render to it directly
		}
		m_ImageToRender = std::make_unique<Image>(w, h, ImageFormat::RGBA);
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

		// Treat the requested width and height as logical size: on Windows and X11 the window is scaled by the monitor's
		// content scale, and in browsers the canvas is rendered at device pixel resolution.
		glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

		auto* windowHandle = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Name.c_str(), NULL, NULL);
		if (!windowHandle)
		{
			std::cerr << "Could not create GLFW Window!\n";
			assert(false);
			return;
		}

		m_RenderingBackend->Init(windowHandle, m_Specification.EnableGraphicsValidation);

		glfwSetWindowUserPointer(windowHandle, this);
		glfwSetWindowSizeCallback(windowHandle, [](GLFWwindow* win, int width, int height) {
			auto app = static_cast<Application*>(glfwGetWindowUserPointer(win));
			assert(app);
			std::cout << "Resized window to: x=" << width << ", y=" << height << std::endl;
			app->OnWindowResize(win);
		});
		glfwSetWindowContentScaleCallback(windowHandle, [](GLFWwindow* win, float, float) {
			auto app = static_cast<Application*>(glfwGetWindowUserPointer(win));
			assert(app);
			app->OnUIScaleChanged(); // e.g. the window moved to a monitor with a different scale
		});

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
			m_RenderingBackend->CreateMainImagePipeline(m_ImageToRender);
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

		glfwDestroyWindow(m_RenderingBackend->GetWindowHandle());
		glfwTerminate();

		g_ApplicationRunning = false;
	}

	float Application::QueryUIScale() const
	{
		// The scale the desktop asks UI to be drawn at, in window coordinates. Where window coordinates are already
		// logical points (macOS, Wayland, browsers) the framebuffer is larger than the window and ImGui applies that
		// ratio through io.DisplayFramebufferScale, so it is divided out here to avoid scaling twice.
		WalnutWindowHandleType* window = m_RenderingBackend->GetWindowHandle();
		float xscale = 1.0f, yscale = 1.0f;
		glfwGetWindowContentScale(window, &xscale, &yscale);

		int windowWidth = 0, framebufferWidth = 0;
		glfwGetWindowSize(window, &windowWidth, nullptr);
		glfwGetFramebufferSize(window, &framebufferWidth, nullptr);
		const float framebufferScale = (windowWidth > 0 && framebufferWidth > 0) ? float(framebufferWidth) / float(windowWidth) : 1.0f;

		return xscale / framebufferScale;
	}

	void Application::MainLoop()
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		if (glfwWindowShouldClose(m_RenderingBackend->GetWindowHandle())) {
			m_Running = false;
		}

		LayerStackOnUpdate();

		// Resize swap chain?
		if (m_RenderingBackend->NeedToResizeWindow())
		{
			int width, height;
			glfwGetFramebufferSize(m_RenderingBackend->GetWindowHandle(), &width, &height);
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
			main_is_minimized = glfwGetWindowAttrib(m_RenderingBackend->GetWindowHandle(), GLFW_ICONIFIED);
		}

		if (!main_is_minimized) {
			m_RenderingBackend->FrameBegin();
			if (m_Specification.UseImGui) {
				ImDrawData* main_draw_data = ImGui::GetDrawData();
				m_RenderingBackend->FrameRenderImGui(main_draw_data);
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
				auto* backupPtr = glfwGetCurrentContext();  // save currentcontext and have to call glfwMakeContextCurrent later
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backupPtr); // if we do not do this there will be a bug in opengl when docking
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

#ifdef __EMSCRIPTEN__
    	emscripten_set_main_loop_arg(&EmscriptenMainLoop, this, 0, true);
#else
		// Main loop
		while (m_Running)
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
