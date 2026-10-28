#include "Application.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include <algorithm>
#include <iostream>
#include <thread>
#include <imgui.h>
#include <imgui_internal.h> // DockBuilder

#include "RenderingBackend.h"
#include "Image.h"

namespace Walnut {

	
void Application::PushLayer(const std::shared_ptr<Layer>& layer)
{
	if (!layer)
		return;

	m_LayerChanges.push_back({ LayerChange::Type::Push, layer });
	ApplyLayerChangesIfNotRunning();
}

void Application::PopLayer()
{
	m_LayerChanges.push_back({ LayerChange::Type::PopTop, nullptr });
	ApplyLayerChangesIfNotRunning();
}

void Application::PopLayer(const std::shared_ptr<Layer>& layer)
{
	if (!layer)
		return;

	// Pushed but not attached yet: dropped, so the layer never gets OnAttach
	auto pending = std::find_if(m_LayerChanges.begin(), m_LayerChanges.end(), [&](const LayerChange& change)
	{
		return change.Action == LayerChange::Type::Push && change.Target == layer;
	});
	if (pending != m_LayerChanges.end())
	{
		m_LayerChanges.erase(pending);
		return;
	}

	m_LayerChanges.push_back({ LayerChange::Type::Pop, layer });
	ApplyLayerChangesIfNotRunning();
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
		// The node exists once imgui.ini restored it or the default layout was built.
		if (m_DockLayoutCallback && ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
		{
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetContentRegionAvail());
			m_DockLayoutCallback(dockspace_id);
			ImGui::DockBuilderFinish(dockspace_id);
		}
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

void Application::SetDefaultDockLayoutCallback(const std::function<void(uint32_t dockspaceId)>& dockLayoutCallback)
{
	m_DockLayoutCallback = dockLayoutCallback;
}

void Application::CaptureNextFrame(const FrameCaptureCallback& callback)
{
	m_FrameCaptureCallback = callback;
}

void Application::CaptureFrameIfRequested()
{
	if (!m_FrameCaptureCallback)
		return;

	// Cleared before the call so the callback can request another capture.
	FrameCaptureCallback callback = std::move(m_FrameCaptureCallback);
	m_FrameCaptureCallback = nullptr;

	std::vector<uint8_t> pixels;
	uint32_t width = 0, height = 0;
	if (m_RenderingBackend->ReadFramePixels(pixels, width, height))
		callback(pixels.data(), width, height);
	else
		callback(nullptr, 0, 0);
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

std::unique_ptr<Image>& Application::MainImageRef()
{
    return m_ImageToRender;
}

void Application::ApplyLayerChangesIfNotRunning()
{
	// Before Run() there is no frame to wait for, so a layer is attached as soon as it is pushed
	if (!m_Running)
		ApplyLayerChanges();
}

void Application::ApplyLayerChanges()
{
	// Taken out first: layers that push or pop from OnAttach or OnDetach queue changes for the next frame
	std::vector<LayerChange> changes;
	changes.swap(m_LayerChanges);

	// Pops first, so a layer is detached before its replacement is attached
	bool gpuIdle = false;
	for (LayerChange& change : changes)
	{
		if (change.Action == LayerChange::Type::Push)
			continue;

		// Not kept in the change, so the layer can be destroyed right after it is detached
		std::shared_ptr<Layer> target = std::move(change.Target);
		auto it = m_LayerStack.end();
		if (change.Action == LayerChange::Type::Pop)
			it = std::find(m_LayerStack.begin(), m_LayerStack.end(), target);
		else if (!m_LayerStack.empty())
			it = std::prev(m_LayerStack.end());
		target.reset();
		if (it == m_LayerStack.end())
			continue;

		// The last frames may still be rendering with the layer's resources, e.g. an image drawn through ImGui.
		// Once per batch is enough: the uploads and frees done by OnDetach wait for the GPU themselves.
		if (!gpuIdle)
		{
			m_RenderingBackend->WaitIdle();
			gpuIdle = true;
		}
		std::shared_ptr<Layer> layer = std::move(*it);
		m_LayerStack.erase(it);
		layer->OnDetach();
		// layer is released here, so it is destroyed before the next change unless the client still holds it
	}

	for (LayerChange& change : changes)
	{
		if (change.Action != LayerChange::Type::Push)
			continue;

		m_LayerStack.push_back(change.Target);
		change.Target->OnAttach();
	}
}

void Application::LayerStackOnUpdate()
{
	ApplyLayerChanges();

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
	// Layers still queued were never attached, so they are not detached either
	m_LayerChanges.clear();

	if (!m_LayerStack.empty())
		m_RenderingBackend->WaitIdle();

	std::vector<std::shared_ptr<Layer>> layers = std::move(m_LayerStack);
	m_LayerStack.clear();
	for (auto& layer : layers)
		layer->OnDetach();
	// Changes queued by OnDetach have nothing left to apply to
	m_LayerChanges.clear();
	for (auto& layer : layers)
		layer.reset();
}

}
