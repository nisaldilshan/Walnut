#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include <Walnut/Timer.h>
#include <Walnut/Image.h>
#include <GLFW/glfw3.h>

#include "../Common/Renderer2D.h"
#include "FluidSolver2D.h"

/**
 * A structure that describes the data layout in the vertex buffer
 * We do not instantiate it but use it in `sizeof` and `offsetof`
 */
struct VertexAttributes {
	glm::vec2 position;
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
			@location(0) position: vec2f,
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
			out.position = vec4f(in.position, 0.0, 1.0);
			out.uv = in.uv;
			return out;
		}

		fn hash22(p: vec2u) -> vec2u {
			var v = p * 1664525u + 1013904223u;
			v.x += v.y * 1664525u; v.y += v.x * 1664525u;
			v ^= v >> vec2u(16u);
			v.x += v.y * 1664525u; v.y += v.x * 1664525u;
			v ^= v >> vec2u(16u);
			return v;
		}

		fn rand22(f: vec2f) -> vec2f { return vec2f(hash22(bitcast<vec2u>(f))) / f32(0xffffffff); }

		fn noise2(n: vec2f) -> f32 {
			let d = vec2f(0., 1.);
			let b = floor(n);
			let f = smoothstep(vec2f(0.), vec2f(1.), fract(n));

			let mix1 = mix(rand22(b), rand22(b + d.yx), f.x);
			let mix2 = mix(rand22(b + d.xx), rand22(b + d.yy), f.x);
			let finalmix = mix(mix1, mix2, f.y);
			return (finalmix.x + finalmix.y)/2;
		}

		@fragment
		fn fs_main(in: VertexOutput) -> @location(0) vec4f {
			let noise_1 = noise2(vec2f(in.position.x + uMyUniforms.time*10000, in.position.y + uMyUniforms.time*30000));
			let noise_2 = noise2(vec2f(in.position.x + uMyUniforms.time*20000, in.position.y + uMyUniforms.time*10000));
			let noise_3 = noise2(vec2f(in.position.x + uMyUniforms.time*30000, in.position.y + uMyUniforms.time*20000));
			return vec4f(noise_1, noise_2, noise_3, 1.0);
		}

		)";
		m_renderer->SetShader(shaderSource);

		m_finalImage = std::make_shared<Walnut::Image>(1, 1, Walnut::ImageFormat::RGBA);
		m_fluid = std::make_unique<FluidPlane>(256);
		m_solver = std::make_unique<FluidSolver2D>();
	}

	virtual void OnDetach() override
	{

	}

	void GPUSolve()
	{
		// uint32_t renderWidth = m_viewportWidth;
		// uint32_t renderHeight = m_viewportHeight;
		uint32_t renderWidth = 256;
		uint32_t renderHeight = 256;

		if (!m_renderer ||
            renderWidth != m_renderer->GetWidth() ||
            renderHeight != m_renderer->GetHeight())
        {
			m_renderer->OnResize(renderWidth, renderHeight);

			// Vertex buffer
			// There are 2 floats per vertex, one for x and one for y.
			// But in the end this is just a bunch of floats to the eyes of the GPU,
			// the *layout* will tell how to interpret this.
			const std::vector<float> vertexData = {
				-0.999, -0.999, -1.0 , -1.0,
				+0.999, -0.999, 1.0 , -1.0,
				+0.999, +0.999, 1.0 , 1.0,

				-0.999, -0.999, -1.0 , -1.0,
				-0.999, +0.999, -1.0 , 1.0,
				+0.999, +0.999, 1.0 , 1.0,
			};

			std::vector<wgpu::VertexAttribute> vertexAttribs(2);

			// Position attribute
			vertexAttribs[0].shaderLocation = 0;
			vertexAttribs[0].format = wgpu::VertexFormat::Float32x2;
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
			// Create binding layout (don't forget to = Default)
			wgpu::BindGroupLayoutEntry bGLayoutEntry = wgpu::Default;
			// The binding index as used in the @binding attribute in the shader
			bGLayoutEntry.binding = 0;
			// The stage that needs to access this resource
			bGLayoutEntry.visibility = wgpu::ShaderStage::Vertex;
			bGLayoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
			bGLayoutEntry.buffer.minBindingSize = sizeof(MyUniforms);

			// Create binding layouts
			std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(1, wgpu::Default);
			// The uniform buffer binding that we already had
			wgpu::BindGroupLayoutEntry& uniformBindingLayout = bindingLayoutEntries[0];
			uniformBindingLayout.setDefault();
			uniformBindingLayout.binding = 0;
			uniformBindingLayout.visibility = wgpu::ShaderStage::Fragment;
			uniformBindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
			uniformBindingLayout.buffer.minBindingSize = sizeof(MyUniforms);
			uniformBindingLayout.buffer.hasDynamicOffset = false;

			m_renderer->SetBindGroupLayoutEntry(uniformBindingLayout);
			m_renderer->CreateUniformBuffer(1, sizeof(MyUniforms));

			m_renderer->Init();
        }

		if (m_renderer)
		{
			m_renderer->BeginRenderPass();

			const float time = static_cast<float>(glfwGetTime());
			m_myUniformData.time = time;
			m_renderer->SetUniformBufferData(&m_myUniformData, 0);
       		m_renderer->Render();
			m_renderer->EndRenderPass();
		}
	}

	void CPUSolve()
	{
		// uint32_t renderWidth = m_viewportWidth;
		// uint32_t renderHeight = m_viewportHeight;
		uint32_t renderWidth = 256;
		uint32_t renderHeight = 256;
		if (!m_imageData)
		{
			m_imageData = new uint32_t[renderWidth * renderHeight];
			m_finalImage->Resize(renderWidth, renderHeight);
		}

		if (renderWidth != m_finalImage->GetWidth() || renderHeight != m_finalImage->GetHeight())
		{
			delete[] m_imageData;
			m_imageData = new uint32_t[renderWidth * renderWidth];
			m_finalImage->Resize(renderWidth, renderWidth);
		}

		m_solver->FluidPlaneAddDensity(*m_fluid, 128, 128, 500.0f);
		m_solver->FluidPlaneAddDensity(*m_fluid, 126, 128, 500.0f);
		m_solver->FluidPlaneAddDensity(*m_fluid, 130, 128, 500.0f);
		m_solver->FluidPlaneAddDensity(*m_fluid, 128, 126, 500.0f);
		m_solver->FluidPlaneAddDensity(*m_fluid, 128, 130, 500.0f);

		m_solver->FluidPlaneAddVelocity(*m_fluid, 110, 110, 1.0f, 1.0f);
		m_solver->FluidPlaneAddVelocity(*m_fluid, 110, 110, 2.0f, 3.0f);
		m_solver->FluidPlaneAddVelocity(*m_fluid, 110, 110, 3.0f, 2.0f);

		m_solver->FluidSolveStep(*m_fluid);

		for (size_t i = 0; i < renderWidth * renderWidth; i++)
		{
			uint8_t red = m_fluid->density[i];
			uint8_t green = m_fluid->density[i];
			uint8_t blue = m_fluid->density[i];
			constexpr uint8_t alpha = 255;
        	uint32_t result = (alpha << 24) | (blue << 16) | (green << 8) | red;
			m_imageData[i] = result;
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
	std::unique_ptr<FluidPlane> m_fluid;
	std::unique_ptr<FluidSolver2D> m_solver;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Renderer3D Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<Renderer2DLayer>();
	return app;
}