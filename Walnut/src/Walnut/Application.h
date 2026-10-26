#pragma once

#include "Layer.h"

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>

#include "ExportConfig.h"
#include "WindowHandle.h"
#include "ImageFormat.h"

Walnut_API extern bool g_ApplicationRunning;

namespace Walnut {

	class RenderingBackend;
	class Image;

	struct ApplicationSpecification
	{
		std::string Name = "Walnut App";
		uint32_t Width = 1600;
		uint32_t Height = 900;
		bool UseImGui = true;
		// Vulkan: turns on VK_LAYER_KHRONOS_validation + VK_EXT_debug_utils when the layer is
		// installed; a no-op otherwise. Ignored by the OpenGL and WebGPU backends.
		bool EnableGraphicsValidation = false;
	};

	// Receives a frame read back from the main window: width * height RGBA8 pixels with the top row first,
	// or a null pixels pointer if the renderer could not read the frame back.
	using FrameCaptureCallback = std::function<void(const uint8_t* pixels, uint32_t width, uint32_t height)>;

	class Walnut_API Application
	{
	public:
		Application(const ApplicationSpecification& applicationSpecification = ApplicationSpecification());
		~Application();
		static Application& Get();

		void MainLoop();
		void Run();
		void SetMenubarCallback(const std::function<void()>& menubarCallback);
		// Called when the main dockspace has no saved layout (e.g. no imgui.ini yet), before the layers draw.
		// dockspaceId is the dockspace's ImGuiID: split it and dock windows into it with ImGui's DockBuilder API.
		void SetDefaultDockLayoutCallback(const std::function<void(uint32_t dockspaceId)>& dockLayoutCallback);
		// Reads back the next frame rendered to the main window, before it is presented.
		void CaptureNextFrame(const FrameCaptureCallback& callback);

		template<typename T>
		void PushLayer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			m_NewLayers.emplace_back(std::make_shared<T>())->OnAttach();
		}

		void PushLayer(const std::shared_ptr<Layer>& layer);
		void PopLayer();

		void Close();
		float GetTime();
		WalnutWindowHandleType* GetWindowHandle() const;

		void SetSleepAmount(std::chrono::milliseconds sleepAmount);
		std::unique_ptr<Image>& MainImageRef();

	private:
		void Init();
		void Shutdown();
		void InitImGui();
		void SetupImGuiForOneIteration();
		void OnWindowResize(WalnutWindowHandleType *win);
		void LayerStackOnUpdate();
		void LayerStackOnGui();
		void LayerStackShutdown();
		void CaptureFrameIfRequested();

		ApplicationSpecification m_Specification;
		std::unique_ptr<RenderingBackend> m_RenderingBackend;
		bool m_Running = false;
		float m_TimeStep = 0.0f;
		std::chrono::time_point<std::chrono::steady_clock> m_LastFrameTimePoint;
		std::chrono::milliseconds m_SleepAmount;
		std::unique_ptr<Image> m_ImageToRender;

		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		std::vector<std::shared_ptr<Layer>> m_NewLayers;
		uint32_t m_LayerPopCount = 0;

		std::function<void()> m_MenubarCallback;
		std::function<void(uint32_t)> m_DockLayoutCallback;
		FrameCaptureCallback m_FrameCaptureCallback;
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}