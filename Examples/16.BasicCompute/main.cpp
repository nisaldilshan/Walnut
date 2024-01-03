#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include <Walnut/Timer.h>

#include "../Common/Compute.h"
#include <GLFW/glfw3.h>

#include <Walnut/GLM/GLM.h>

class Renderer2DLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		m_compute.reset();
		m_compute = std::make_unique<Compute>();
		m_compute->Init();
	}

	virtual void OnDetach() override
	{}

	virtual void OnUpdate(float ts) override
	{
        Walnut::Timer timer;

		m_compute->BeginComputePass();

		m_compute->EndComputePass();

        m_lastRenderTime = timer.ElapsedMillis();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms", m_lastRenderTime);
		ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Results");
		ImGui::End();
        ImGui::PopStyleVar();

		ImGui::ShowDemoWindow();
	}

private:
    std::unique_ptr<Compute> m_compute;
    float m_lastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Compute Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<Renderer2DLayer>();
	return app;
}