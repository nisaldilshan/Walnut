#pragma once

#include "Layer.h"

#include <string>
#include <vector>
#include <memory>
#include <utility>
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

		// Layers can be pushed and popped at any time, also from a layer's OnUpdate or OnUIRender. While the
		// application runs, the changes are queued and applied at the start of the next frame, before any layer
		// updates: first every pop, in order, then every push, in order. A popped layer gets OnDetach after the GPU
		// has finished every frame that used it. While the application is not running, e.g. before Run(), the changes are
		// applied at once.
		template<typename T, typename... Args>
		std::shared_ptr<T> PushLayer(Args&&... args)
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			std::shared_ptr<T> layer = std::make_shared<T>(std::forward<Args>(args)...);
			PushLayer(layer);
			return layer;
		}

		// A null layer is ignored.
		void PushLayer(const std::shared_ptr<Layer>& layer);
		// Removes the topmost layer that is already attached. Layers pushed in the same frame are not counted.
		void PopLayer();
		// Removes the given layer. A layer that is pushed but not attached yet is dropped without ever being
		// attached. A null layer is ignored.
		void PopLayer(const std::shared_ptr<Layer>& layer);

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
		void ApplyLayerChanges();
		void ApplyLayerChangesIfNotRunning();
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

		// A queued pop, or push of Target. PopTop has no target and removes the topmost attached layer.
		struct LayerChange
		{
			enum class Type { Push, Pop, PopTop };
			Type Action;
			std::shared_ptr<Layer> Target;
		};

		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		std::vector<LayerChange> m_LayerChanges;

		std::function<void()> m_MenubarCallback;
		std::function<void(uint32_t)> m_DockLayoutCallback;
		FrameCaptureCallback m_FrameCaptureCallback;
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}