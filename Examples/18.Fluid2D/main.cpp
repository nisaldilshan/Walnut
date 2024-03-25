#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include <Walnut/Timer.h>
#include <Walnut/Image.h>
#include <GLFW/glfw3.h>

#include "../Common/Renderer2D.h"

/**
 * A structure that describes the data layout in the vertex buffer
 * We do not instantiate it but use it in `sizeof` and `offsetof`
 */
struct VertexAttributes {
	glm::vec3 position;
	glm::vec2 uv;
};

struct MyUniforms {
	glm::vec3 cameraWorldPosition;
    float time;
};
static_assert(sizeof(MyUniforms) % 16 == 0);

class Renderer2DLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		m_renderer.reset();
		m_renderer = std::make_shared<Renderer2D>();

		const char* shaderSource = R"(
		struct VertexInput {
			@location(0) position: vec3f,
			@location(1) uv: vec2f,
		};

		struct VertexOutput {
			@builtin(position) position: vec4f,
			@location(0) uv: vec2f,
		};

		struct MyUniforms {
			cameraWorldPosition: vec3f,
			time: f32,
		};

		@group(0) @binding(0) var<uniform> uMyUniforms: MyUniforms;

		@vertex
		fn vs_main(in: VertexInput) -> VertexOutput {
			var out: VertexOutput;
			out.position = vec4f(in.position, 1.0);
			out.uv = in.uv;
			return out;
		}

		@fragment
		fn fs_main(in: VertexOutput) -> @location(0) vec4f {
			return vec4f(1.0, 1.0, 1.0, 1.0);
		}

		)";
		m_renderer->SetShader(shaderSource);

		m_finalImage = std::make_shared<Walnut::Image>(1, 1, Walnut::ImageFormat::RGBA);
	}

	virtual void OnDetach() override
	{

	}

	void GPUSolve()
	{
		if (!m_renderer ||
            m_viewportWidth != m_renderer->GetWidth() ||
            m_viewportHeight != m_renderer->GetHeight())
        {
			m_renderer->OnResize(m_viewportWidth, m_viewportHeight);

			// Vertex buffer
			// There are 2 floats per vertex, one for x and one for y.
			// But in the end this is just a bunch of floats to the eyes of the GPU,
			// the *layout* will tell how to interpret this.
			const std::vector<float> vertexData = {
				-0.999, -0.999, 0.0, -1.0 , -1.0,
				+0.999, -0.999, 0.0, 1.0 , -1.0,
				+0.999, +0.999, 0.0, 1.0 , 1.0,

				-0.999, -0.999, 0.0, -1.0 , -1.0,
				-0.999, +0.999, 0.0, -1.0 , 1.0,
				+0.999, +0.999, 0.0, 1.0 , 1.0,
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

			// Create binding layouts
			std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(1, wgpu::Default);
			// The uniform buffer binding that we already had
			wgpu::BindGroupLayoutEntry& uniformBindingLayout = bindingLayoutEntries[0];
			uniformBindingLayout.setDefault();
			uniformBindingLayout.binding = 0;
			uniformBindingLayout.visibility = wgpu::ShaderStage::Fragment;
			uniformBindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
			uniformBindingLayout.buffer.minBindingSize = sizeof(MyUniforms);
			uniformBindingLayout.buffer.hasDynamicOffset = true;

			//m_renderer->CreateUniformBuffer(1, UniformBuf::UniformType::ModelViewProjection, sizeof(MyUniforms), uniformBindingLayout.binding);
			//m_renderer->CreateBindGroup(bindingLayoutEntries);

			m_renderer->Init();
        }

		if (m_renderer)
		{
			m_renderer->BeginRenderPass();

			const float time = static_cast<float>(glfwGetTime());
			m_myUniformData.time = time;
			// m_renderer->SetUniformBufferData(UniformBuf::UniformType::ModelViewProjection, &m_myUniformData, 0);
       		m_renderer->Render();
			m_renderer->EndRenderPass();
		}
	}

	void CPUSolve()
	{
		if (!m_imageData)
		{
			m_imageData = new uint32_t[m_viewportWidth * m_viewportHeight];
			m_finalImage->Resize(m_viewportWidth, m_viewportHeight);
		}

		if (m_viewportWidth != m_finalImage->GetWidth() || m_viewportHeight != m_finalImage->GetHeight())
		{
			delete[] m_imageData;
			m_imageData = new uint32_t[m_viewportWidth * m_viewportWidth];
			m_finalImage->Resize(m_viewportWidth, m_viewportWidth);
		}

		for (size_t i = 0; i < m_viewportWidth * m_viewportWidth; i++)
		{
			m_imageData[i] = Walnut::Random::UInt();
			m_imageData[i] |= 0xff000000; // remove randomnes from alpha channel
		}
		m_finalImage->SetData(m_imageData);
	}

	virtual void OnUpdate(float ts) override
	{
        Walnut::Timer timer;
		if (m_viewportWidth == 0 || m_viewportHeight == 0)
			return;

        if (m_hWSolver)
		{
			GPUSolve();
		}
		else
		{
			CPUSolve();
		}

        m_lastRenderTime = timer.ElapsedMillis();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms", m_lastRenderTime);


		ImGui::Checkbox("HW", &m_hWSolver);

		ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
		m_viewportWidth = ImGui::GetContentRegionAvail().x;
        m_viewportHeight = ImGui::GetContentRegionAvail().y;

		if (m_hWSolver)
		{
			if (m_renderer)
            	ImGui::Image(m_renderer->GetDescriptorSet(), {(float)m_renderer->GetWidth(),(float)m_renderer->GetWidth()});
			else
				assert(false);
		}
		else
		{
			if (m_finalImage)
			{
				ImVec2 uv_min = ImVec2(1, 0);                 // Top-left
				ImVec2 uv_max = ImVec2(0, 1); 
				float aspectRatio = (float)m_finalImage->GetWidth() / (float)m_finalImage->GetHeight();
				float viewHeight = (float)m_finalImage->GetHeight();
				ImGui::Image((void*)m_finalImage->GetDescriptorSet(), { aspectRatio * viewHeight, viewHeight }, uv_min, uv_max);
			}
			else
				assert(false);
		}
        
		ImGui::End();
        ImGui::PopStyleVar();

		ImGui::ShowDemoWindow();
	}

private:
    std::shared_ptr<Renderer2D> m_renderer;
    uint32_t m_viewportWidth = 0;
    uint32_t m_viewportHeight = 0;
    float m_lastRenderTime = 0.0f;
	MyUniforms m_myUniformData;

	bool m_hWSolver = false;
	uint32_t* m_imageData = nullptr;
	std::shared_ptr<Walnut::Image> m_finalImage = nullptr;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Renderer3D Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<Renderer2DLayer>();
	return app;
}