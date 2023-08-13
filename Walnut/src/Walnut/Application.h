#pragma once

#include "Layer.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "imgui.h"
#include <vulkan/vulkan.h>

#ifdef __EMSCRIPTEN__
#include <webgpu/webgpu_cpp.h>
#endif

#include "RenderingBackend.h"

struct GLFWwindow;

namespace Walnut {

	struct ApplicationSpecification
	{
		std::string Name = "Walnut App";
		uint32_t Width = 1600;
		uint32_t Height = 900;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& applicationSpecification = ApplicationSpecification());
		~Application();

		static Application& Get();

		void Run();
		void SetMenubarCallback(const std::function<void()>& menubarCallback);
		void SetUIRenderingCallback();
		
		template<typename T>
		void PushLayer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
		}

		void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.emplace_back(layer); layer->OnAttach(); }

		void Close();

		float GetTime();
		GLFWwindow* GetWindowHandle() const;

		static VkInstance GetInstance1();
		static VkPhysicalDevice GetPhysicalDevice1();
		static VkDevice GetDevice1();

		static VkCommandBuffer GetGraphicsCommandBuffer(bool begin);
		static void FlushGraphicsCommandBuffer(VkCommandBuffer commandBuffer);

		static void SubmitGraphicsResourceFree(); // std::function<void()>&& func
	private:
		void Init();
		void Shutdown();
	private:
		ApplicationSpecification m_Specification;
		std::unique_ptr<RenderingBackend> m_RenderingBackend = nullptr;
		bool m_Running = false;

		float m_TimeStep = 0.0f;
		float m_FrameTime = 0.0f;
		float m_LastFrameTime = 0.0f;

		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		std::function<void()> m_MenubarCallback;
		std::function<void()> m_UIRenderingCallback;

#ifdef __EMSCRIPTEN__
		wgpu::Device device_;
    	wgpu::Queue queue_;
#endif
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}