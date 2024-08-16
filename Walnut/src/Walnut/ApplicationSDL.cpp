#include "Application.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include <iostream>
#include <thread>
#include <imgui.h>

#include <Walnut/GLM/GLM.h>
#include <imgui_impl_sdl2.h>

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

	void Application::OnWindowResize(WindowHandleType *win, int width, int height)
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
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "Could not initalize GLFW!\n";
			assert(false);
			return;
		}

		if (RenderingBackend::GetBackend() == RenderingBackend::BACKEND::OpenGL)
		{
#if defined(__EMSCRIPTEN__)
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#elif defined(__ANDROID__)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#else
			//SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#endif

			// glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
			// glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			// glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		}
		else if (RenderingBackend::GetBackend() == RenderingBackend::BACKEND::Vulkan)
		{
			//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		else if (RenderingBackend::GetBackend() == RenderingBackend::BACKEND::WebGPU)
		{
			// glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			// glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		}
		else
		{
			assert(false);
		}

        auto* windowHandle = SDL_CreateWindow(m_Specification.Name.c_str(), 
											SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
											m_Specification.Width, m_Specification.Height, 
											SDL_WINDOW_OPENGL);
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

		// Create Framebuffers
		{
			// int w, h;
			// glfwGetFramebufferSize(windowHandle, &w, &h);

            int w,h;
            SDL_GetWindowSize(windowHandle, &w, &h);
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
		for (auto& layer : m_LayerStack)
			layer->OnDetach();

		m_LayerStack.clear();

		m_RenderingBackend->Shutdown();

		//ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		m_RenderingBackend->Cleanup();

		// glfwDestroyWindow(m_RenderingBackend->GetWindowHandle());
		// glfwTerminate();

		g_ApplicationRunning = false;
	}

	void Application::SetupImGuiForOneIteration()
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

		for (auto& layer : m_LayerStack)
			layer->OnUIRender();

		ImGui::End();
	}

    void updateGui() 
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
            ImGui_ImplSDL2_ProcessEvent(&event);
            // if (event.type == SDL_QUIT)
            //     done = true;
            // if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            //     done = true;
        }

		for (auto& layer : m_LayerStack)
			layer->OnUpdate(m_TimeStep);

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
			//auto* backupPtr = glfwGetCurrentContext();  // save currentcontext and have to call glfwMakeContextCurrent later
            auto* backupPtr = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			//glfwMakeContextCurrent(backupPtr); // if we do not do this there will be a bug in opengl when docking
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

    void Application::SetMenubarCallback(const std::function<void()> &menubarCallback)
    {
		m_MenubarCallback = menubarCallback; 
    }


    void Application::Close()
	{
		m_Running = false;
	}

	float Application::GetTime()
	{
		//return (float)glfwGetTime();
        return (float)0;
	}

    WindowHandleType* Application::GetWindowHandle() const
    {
        return m_RenderingBackend->GetWindowHandle();
    }

    void Application::SetSleepAmount(std::chrono::milliseconds sleepAmount)
    {
		m_SleepAmount = sleepAmount;
    }
}