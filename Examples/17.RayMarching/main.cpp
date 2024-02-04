#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include <Walnut/Timer.h>

#include "../Common/Renderer3D.h"

/**
 * A structure that describes the data layout in the vertex buffer
 * We do not instantiate it but use it in `sizeof` and `offsetof`
 */
struct VertexAttributes {
	glm::vec3 position;
	glm::vec2 uv;
};

class Renderer3DLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		m_renderer.reset();
		m_renderer = std::make_shared<Renderer3D>();
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
			m_renderer->OnResize(m_viewportWidth, m_viewportHeight);

			const char* shaderSource = R"(
			
			struct VertexInput {
				@location(0) position: vec3f,
				@location(1) uv: vec2f,
			};

			struct VertexOutput {
				@builtin(position) position: vec4f,
				@location(0) uv: vec2f,
			};

			@vertex
			fn vs_main(in: VertexInput) -> VertexOutput {
				var out: VertexOutput;
				out.position = vec4f(in.position, 1.0);
				out.uv = in.uv;
				return out;
			}

			@fragment
			fn fs_main(in: VertexOutput) -> @location(0) vec4f {
				//let cameraPos = vec3f(0.0, 1.82, 0.83);
				let cameraPos = vec3f(0.0, 0.0, -3.0); // ray origin
				let rayDirection = normalize(vec3f(in.uv, 1.0));

				var t = 0.0; // total distance travelled form cameras ray origin
				var color = vec3f(0.0, 0.0, 0.0);
				for (var i = 0; i < 80; i++)
				{
					let p = cameraPos + rayDirection * t;
					//float d = map(p);
					let d = length(p) - 0.3;
					t = t + d;

					let one = f32(i);
					color = vec3f(one, one, one) / 80.0;

					if d < 0.001
					{
						break;
					}
				}
				return vec4f(color, 1.0);
			}

			)";
			m_renderer->SetShader(shaderSource);

			// Vertex buffer
			// There are 2 floats per vertex, one for x and one for y.
			// But in the end this is just a bunch of floats to the eyes of the GPU,
			// the *layout* will tell how to interpret this.
			const std::vector<float> vertexData = {
				-0.9, -0.9, 0.0, -1.0 , -1.0,
				+0.9, -0.9, 0.0, 1.0 , -1.0,
				+0.9, +0.9, 0.0, 1.0 , 1.0,

				-0.9, -0.9, 0.0, -1.0 , -1.0,
				-0.9, +0.9, 0.0, -1.0 , 1.0,
				+0.9, +0.9, 0.0, 1.0 , 1.0,
			};

			std::vector<wgpu::VertexAttribute> vertexAttribs(2);

			// Position attribute
			vertexAttribs[0].shaderLocation = 0;
			vertexAttribs[0].format = wgpu::VertexFormat::Float32x3;
			vertexAttribs[0].offset = 0;

			// UV attribute
			vertexAttribs[1].shaderLocation = 1;
			vertexAttribs[1].format = wgpu::VertexFormat::Float32x2;
			vertexAttribs[1].offset = offsetof(VertexAttributes, uv);

			wgpu::VertexBufferLayout vertexBufferLayout;
			vertexBufferLayout.attributeCount = (uint32_t)vertexAttribs.size();
			vertexBufferLayout.attributes = vertexAttribs.data();
			vertexBufferLayout.arrayStride = sizeof(VertexAttributes);
			vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;


			m_renderer->SetVertexBufferData(vertexData.data(), vertexData.size() * 4, vertexBufferLayout);

			m_renderer->Init();
        }

		if (m_renderer)
		{
			m_renderer->BeginRenderPass();
       		m_renderer->Render(0);
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
    std::shared_ptr<Renderer3D> m_renderer;
    uint32_t m_viewportWidth = 0;
    uint32_t m_viewportHeight = 0;
    float m_lastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Renderer3D Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<Renderer3DLayer>();
	return app;
}