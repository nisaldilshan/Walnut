#pragma once

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5_webgpu.h>
#endif

#include "../RenderingBackend.h"
#include "VulkanGraphics.h"

namespace Walnut {

class GlfwVulkanRenderingBackend : public RenderingBackend
{
public:
	virtual void Init(GLFWwindow* windowHandle) override
	{
		if (!glfwVulkanSupported())
		{
			std::cerr << "GLFW: Vulkan not supported!\n";
			return;
		}

		m_extensions = glfwGetRequiredInstanceExtensions(&m_extensions_count);
		m_windowHandle = windowHandle;
	}

	void SetupGraphicsAPI() override
	{
#ifdef __EMSCRIPTEN__
		SetupWebGPU(m_extensions, m_extensions_count);
#else
		// Setup Vulkan
		GraphicsAPI::Vulkan::SetupVulkan(m_extensions, m_extensions_count);
#endif
		// Create Window Surface
		GraphicsAPI::Vulkan::AddWindowHandle(m_windowHandle);
	}

	void SetupWindow(int width, int height) override
	{
		GraphicsAPI::Vulkan::SetupVulkanWindow(width, height);
	}

	bool NeedToResizeWindow() override
	{
		return GraphicsAPI::Vulkan::NeedSwapChainRebuild();
	}

	void ResizeWindow(int width, int height) override
	{
		GraphicsAPI::Vulkan::ResizeVulkanWindow(width, height);
		GraphicsAPI::Vulkan::SetSwapChainRebuildStatus(false);
	}

	void ConfigureImGui() override
	{
		GraphicsAPI::Vulkan::ConfigureRendererBackend(m_windowHandle);
	}

	void StartImGuiFrame(const std::function<void()>& applicationMenubarCallback, const std::function<void()>& applicationUIRenderingCallback) override
	{
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
			if (applicationMenubarCallback)
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

			if (applicationMenubarCallback)
			{
				if (ImGui::BeginMenuBar())
				{
					applicationMenubarCallback();
					ImGui::EndMenuBar();
				}
			}

			if (applicationUIRenderingCallback)
				applicationUIRenderingCallback();
			else
				assert(false); std::cout << "Error - applicationUIRenderingCallback is not set!" << std::endl;

			ImGui::End();
		}
	}

	void UploadFonts() override
	{
		GraphicsAPI::Vulkan::UploadFonts();
	}

	GLFWwindow* GetWindowHandle() override
	{
		return m_windowHandle;
	}
	
	virtual void Clear() override
	{
		
	}

private:
	uint32_t m_extensions_count = 0;
	const char** m_extensions = nullptr;
	GLFWwindow* m_windowHandle = nullptr;
};


}