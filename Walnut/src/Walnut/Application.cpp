#include "Application.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include <iostream>
#include <thread>
#include <imgui.h>

#include "RenderingBackend.h"

namespace Walnut {

	
void Application::PushLayer(const std::shared_ptr<Layer>& layer) 
{ 
	m_NewLayers.emplace_back(layer); 
	layer->OnAttach(); 
}

void Application::PopLayer()
{
	if (m_LayerStack.size() > 0)
	{
		m_LayerPopCount++;
	}
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

	LayerStackOnGui();

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

void Application::SetMenubarCallback(const std::function<void()> &menubarCallback)
{
	m_MenubarCallback = menubarCallback; 
}

void Application::Close()
{
	m_Running = false;
}

WalnutWindowHandleType* Application::GetWindowHandle() const
{
	return m_RenderingBackend->GetWindowHandle();
}

void Application::SetSleepAmount(std::chrono::milliseconds sleepAmount)
{
	m_SleepAmount = sleepAmount;
}

void Application::LayerStackOnUpdate()
{
	if (m_LayerPopCount > 0)
	{
		for (uint32_t i = 0; i < m_LayerPopCount; i++)
		{
			if (m_LayerStack.size() > 0)
			{
				m_LayerStack.back()->OnDetach();
				m_LayerStack.pop_back();
			}
		}
		m_LayerPopCount = 0;
	}

	if (m_NewLayers.size() > 0)
	{
		std::move(m_NewLayers.begin(), m_NewLayers.end(), std::back_inserter(m_LayerStack));
		m_NewLayers.clear();
	}

	for (auto& layer : m_LayerStack)
		layer->OnUpdate(m_TimeStep);
}

void Application::LayerStackOnGui()
{
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
}

void Application::LayerStackShutdown()
{
	for (auto& layer : m_LayerStack)
		layer->OnDetach();

	m_LayerStack.clear();
}

}
