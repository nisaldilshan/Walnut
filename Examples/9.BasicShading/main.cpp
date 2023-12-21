#define DAWN_DEBUG_BREAK_ON_ERROR 1

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include <Walnut/Timer.h>

#include "../Common/Renderer3D.h"
#include "../Common/Geometry.h"
#include <GLFW/glfw3.h>

#include <Walnut/GLM/GLM.h>

/**
 * A structure that describes the data layout in the vertex buffer
 * We do not instantiate it but use it in `sizeof` and `offsetof`
 */
struct VertexAttributes {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
};

struct MyUniforms {
	// We add transform matrices
    glm::mat4x4 projectionMatrix;
    glm::mat4x4 viewMatrix;
    glm::mat4x4 modelMatrix;
    std::array<float, 4> color;
    float time;
    float _pad[3];
};

glm::mat4x4 makePerspectiveProj(float ratio, float near, float far, float focalLength)
{
	float divides = 1.0f / (far - near);
	float aaa[16] = {
		focalLength,         0.0,              0.0,               0.0,
			0.0,     focalLength * ratio,      0.0,               0.0,
			0.0,             0.0,         far * divides, -far * near * divides,
			0.0,             0.0,              1.0,               0.0,
	};
	return glm::transpose(glm::make_mat4(aaa));
}

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
			m_renderer = std::make_shared<Renderer3D>();
			m_renderer->OnResize(m_viewportWidth, m_viewportHeight);

			const char* shaderSource = R"(
			struct VertexInput {
				@location(0) position: vec3f,
				@location(1) normal: vec3f,
				@location(2) color: vec3f,
			};

			struct VertexOutput {
				@builtin(position) position: vec4f,
				@location(0) color: vec3f,
				@location(1) normal: vec3f,
			};

			/**
			 * A structure holding the value of our uniforms
			 */
			struct MyUniforms {
				projectionMatrix: mat4x4f,
				viewMatrix: mat4x4f,
				modelMatrix: mat4x4f,
				color: vec4f,
				time: f32,
			};

			// Instead of the simple uTime variable, our uniform variable is a struct
			@group(0) @binding(0) var<uniform> uMyUniforms: MyUniforms;

			@vertex
			fn vs_main(in: VertexInput) -> VertexOutput {
				var out: VertexOutput;
				out.position = uMyUniforms.projectionMatrix * uMyUniforms.viewMatrix * uMyUniforms.modelMatrix * vec4f(in.position, 1.0);
				// Forward the normal
				out.normal = (uMyUniforms.modelMatrix * vec4f(in.normal, 0.0)).xyz;
				out.color = in.color;
				return out;
			}

			@fragment
			fn fs_main(in: VertexOutput) -> @location(0) vec4f {
				let normal = normalize(in.normal);

				let lightColor1 = vec3f(1.0, 0.9, 0.6);
				let lightColor2 = vec3f(0.6, 0.9, 1.0);
				let lightDirection1 = vec3f(0.5, -0.9, 0.1);
				let lightDirection2 = vec3f(0.2, 0.4, 0.3);
				let shading1 = max(0.0, dot(lightDirection1, normal));
				let shading2 = max(0.0, dot(lightDirection2, normal));
				let shading = shading1 * lightColor1 + shading2 * lightColor2;
				let color = in.color * shading;

				// Gamma-correction
				let corrected_color = pow(color, vec3f(2.2));
				return vec4f(corrected_color, uMyUniforms.color.a);
			}
			)";
			m_renderer->SetShader(shaderSource);

			//
			std::vector<VertexAttributes> vertexData;
			bool success = Geometry::loadGeometryFromObj<VertexAttributes>(RESOURCE_DIR "/mammoth.obj", vertexData);
			if (!success) 
			{
				std::cerr << "Could not load geometry!" << std::endl;
				return;
			}
			//

			// Vertex fetch
			// We now have 2 attributes
			std::vector<wgpu::VertexAttribute> vertexAttribs(3);

			// Position attribute
			vertexAttribs[0].shaderLocation = 0;
			vertexAttribs[0].format = wgpu::VertexFormat::Float32x3;
			vertexAttribs[0].offset = 0;

			// Normal attribute
			vertexAttribs[1].shaderLocation = 1;
			vertexAttribs[1].format = wgpu::VertexFormat::Float32x3;
			vertexAttribs[1].offset = offsetof(VertexAttributes, normal);

			// Color attribute
			vertexAttribs[2].shaderLocation = 2;
			vertexAttribs[2].format = wgpu::VertexFormat::Float32x3; // different type!
			vertexAttribs[2].offset = offsetof(VertexAttributes, color);

			wgpu::VertexBufferLayout vertexBufferLayout;
			vertexBufferLayout.attributeCount = (uint32_t)vertexAttribs.size();
			vertexBufferLayout.attributes = vertexAttribs.data();
			// stride
			vertexBufferLayout.arrayStride = sizeof(VertexAttributes);
			vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;


			m_renderer->SetVertexBufferData(vertexData.data(), vertexData.size() * sizeof(VertexAttributes), vertexBufferLayout);

			// Create binding layout (don't forget to = Default)
			std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(1, wgpu::Default);
			wgpu::BindGroupLayoutEntry& bGLayoutEntry = bindingLayoutEntries[0];
			// The binding index as used in the @binding attribute in the shader
			bGLayoutEntry.binding = 0;
			// The stage that needs to access this resource
			bGLayoutEntry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
			bGLayoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
			bGLayoutEntry.buffer.minBindingSize = sizeof(MyUniforms);
			// Make this binding dynamic so we can offset it between draw calls
			bGLayoutEntry.buffer.hasDynamicOffset = true;

			m_renderer->SetSizeOfUniform(sizeof(MyUniforms));
			m_renderer->SetBindGroupLayoutEntries(bindingLayoutEntries);

			m_renderer->CreateUniformBuffer(1);

			m_renderer->Init();	
        }

		if (m_renderer)
		{
			m_renderer->BeginRenderPass();

			float time = static_cast<float>(glfwGetTime());

			// Upload first value

			float angle1 = 2.0f;
			constexpr float PI = 3.14159265358979323846f;
			float angle2 = 3.0f * PI / 4.0f;
			glm::vec3 focalPoint(0.0, 0.0, -2.0);			

			glm::mat4x4 V(1.0);
			V = glm::translate(V, -focalPoint);
			V = glm::rotate(V, -angle2, glm::vec3(1.0, 0.0, 0.0));
			m_uniformData.viewMatrix = V;
			
			float focalLength = 2.0;
			float fov = 2 * glm::atan(1 / focalLength);
			float ratio = m_viewportWidth / m_viewportHeight;
			float near = 0.01f;
			float far = 100.0f;
			m_uniformData.projectionMatrix = glm::perspective(fov, ratio, near, far);
			//m_uniformData.projectionMatrix = makePerspectiveProj(m_viewportWidth / m_viewportHeight, 0.01, 100.0, 2.0);

			glm::mat4x4 M1(1.0);
			angle1 = time * 0.9f;
			M1 = glm::rotate(M1, angle1, glm::vec3(0.0, 0.0, 1.0));
			M1 = glm::translate(M1, glm::vec3(0.5, 0.0, 0.0));
			M1 = glm::scale(M1, glm::vec3(0.3f));
			m_uniformData.modelMatrix = M1;

			m_uniformData.time = time; // glfwGetTime returns a double
			m_uniformData.color = { 0.0f, 1.0f, 0.4f, 1.0f };
			m_renderer->SetUniformBufferData(&m_uniformData, 0);
			////

			// Upload second value
			glm::mat4x4 M2(1.0);
			angle1 = time * 1.1f;
			M2 = glm::rotate(M2, angle1, glm::vec3(0.0, 0.0, 1.0));
			M2 = glm::translate(M2, glm::vec3(0.5, 0.0, 0.0));
			M2 = glm::scale(M2, glm::vec3(0.3f));
			m_uniformData.modelMatrix = M2;

			m_uniformData.time = time; // glfwGetTime returns a double
			m_uniformData.color = { 1.0f, 1.0f, 1.0f, 0.7f };
			m_renderer->SetUniformBufferData(&m_uniformData, 1);
			////                         				^^^^^^^^^^^^^ beware of the non-null offset!

			m_renderer->Render(0);
			m_renderer->Render(1);
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

	MyUniforms m_uniformData;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Renderer3D Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<Renderer2DLayer>();
	return app;
}