#pragma once

#include "Layer.h"

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>

#include "ExportConfig.h"
#include "WindowHandle.h"

Walnut_API extern bool g_ApplicationRunning;

namespace Walnut {

	class RenderingBackend;

	struct ApplicationSpecification
	{
		std::string Name = "Walnut App";
		uint32_t Width = 1600;
		uint32_t Height = 900;
	};

	class Walnut_API Application
	{
	public:
		Application(const ApplicationSpecification& applicationSpecification = ApplicationSpecification());
		~Application();
		static Application& Get();

		void MainLoop();
		void Run();
		void SetMenubarCallback(const std::function<void()>& menubarCallback);
		
		template<typename T>
		void PushLayer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
		}

		void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.emplace_back(layer); layer->OnAttach(); }
		void Close();
		float GetTime();
		WindowHandleType* GetWindowHandle() const;

		void SetSleepAmount(std::chrono::milliseconds sleepAmount);

	private:
		void Init();
		void Shutdown();
		void SetupImGuiForOneIteration();
		void OnWindowResize(WindowHandleType *win, int width, int height);

		ApplicationSpecification m_Specification;
		std::unique_ptr<RenderingBackend> m_RenderingBackend;
		bool m_Running = false;
		float m_TimeStep = 0.0f;
		std::chrono::time_point<std::chrono::steady_clock> m_LastFrameTimePoint;
		std::chrono::milliseconds m_SleepAmount;

		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		std::function<void()> m_MenubarCallback;
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}