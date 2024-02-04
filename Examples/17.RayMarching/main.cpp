#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include <Walnut/Timer.h>

#include "../Common/Renderer2D.h"

class Renderer2DLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
	}

	virtual void OnDetach() override
	{

	}

	virtual void OnUpdate(float ts) override
	{
        Walnut::Timer timer;
		if (m_viewportWidth == 0 || m_viewportHeight == 0)
			return;

        if (!m_renderer ||
            m_viewportWidth != m_renderer->GetWidth() ||
            m_viewportHeight != m_renderer->GetHeight())
        {
			m_renderer.reset();
			m_renderer = std::make_shared<Renderer2D>(m_viewportWidth, m_viewportHeight, Walnut::ImageFormat::RGBA);

			const char* shaderSource = R"(
			// The `@location(0)` attribute means that this input variable is described
			// by the vertex buffer layout at index 0 in the `pipelineDesc.vertex.buffers`
			// array.
			// The type `vec2f` must comply with what we will declare in the layout.
			// The argument name `in_vertex_position` is up to you, it is only internal to
			// the shader code!
				@vertex
				fn vs_main(@location(0) in_vertex_position: vec2f) -> @builtin(position) vec4f {
					return vec4f(in_vertex_position, 0.0, 1.0);
				}

				@fragment
				fn fs_main() -> @location(0) vec4f {
					return vec4f(1.0, 0.4, 0.0, 1.0);
				}
			)";
			m_renderer->SetShader(shaderSource);

			// Vertex buffer
			// There are 2 floats per vertex, one for x and one for y.
			// But in the end this is just a bunch of floats to the eyes of the GPU,
			// the *layout* will tell how to interpret this.
			const std::vector<float> vertexData = {
				-0.5, -0.5,
				+0.5, -0.5,
				+0.0, +0.5,

				-0.55f, -0.5,
				-0.05f, +0.5,
				-0.55f, +0.5,

				+0.275f, +0.05,
				+0.5f, +0.5,
				+0.05f, +0.5
			};

			wgpu::VertexBufferLayout vertexBufferLayout;
			wgpu::VertexAttribute vertexAttrib;
			// == Per attribute ==
			// Corresponds to @location(...)
			vertexAttrib.shaderLocation = 0;
			// Means vec2f in the shader
			vertexAttrib.format = wgpu::VertexFormat::Float32x2;
			// Index of the first element
			vertexAttrib.offset = 0;

			vertexBufferLayout.attributeCount = 1;
			vertexBufferLayout.attributes = &vertexAttrib;
			vertexBufferLayout.arrayStride = 2 * sizeof(float);
			vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;


			m_renderer->SetVertexBufferData(vertexData, vertexBufferLayout);

			m_renderer->Init();
        }

		if (m_renderer)
		{
			m_renderer->BeginRenderPass();
       		m_renderer->Render();
			m_renderer->EndRenderPass();
		}

        m_lastRenderTime = timer.ElapsedMillis();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms", m_lastRenderTime);
		ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
		m_viewportWidth = ImGui::GetContentRegionAvail().x;
        m_viewportHeight = ImGui::GetContentRegionAvail().y;
        if (m_renderer)
            ImGui::Image(m_renderer->GetDescriptorSet(), {(float)m_renderer->GetWidth(),(float)m_renderer->GetWidth()});
		ImGui::End();
        ImGui::PopStyleVar();

		ImGui::ShowDemoWindow();
	}

private:
    std::shared_ptr<Renderer2D> m_renderer;
    uint32_t m_viewportWidth = 0;
    uint32_t m_viewportHeight = 0;
    float m_lastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Renderer2D Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<Renderer2DLayer>();
	return app;
}