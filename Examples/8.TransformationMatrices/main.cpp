#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include <Walnut/Timer.h>

#include "../Common/Renderer3D.h"
#include "../Common/Geometry.h"
#include <GLFW/glfw3.h>

#include <glm/ext.hpp>

struct MyUniforms {
	// We add transform matrices
    // glm::mat4x4 projectionMatrix;
    // glm::mat4x4 viewMatrix;
    // glm::mat4x4 modelMatrix;
    std::array<float, 4> color;
    float time;
    float _pad[3];
};

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
			m_renderer = std::make_shared<Renderer3D>(m_viewportWidth, m_viewportHeight, Walnut::ImageFormat::RGBA);

			const char* shaderSource = R"(
			struct VertexInput {
				@location(0) position: vec3f,
				@location(1) color: vec3f,
			};

			struct VertexOutput {
				@builtin(position) position: vec4f,
				@location(0) color: vec3f,
			};

			/**
			 * A structure holding the value of our uniforms
			 */
			struct MyUniforms {
				color: vec4f,
				time: f32,
			};

			// Instead of the simple uTime variable, our uniform variable is a struct
			@group(0) @binding(0) var<uniform> uMyUniforms: MyUniforms;

			const pi = 3.14159265359;

			// Build a perspective projection matrix
			fn makePerspectiveProj(ratio: f32, near: f32, far: f32, focalLength: f32) -> mat4x4f {
				let divides = 1.0 / (far - near);
				return transpose(mat4x4f(
					focalLength,         0.0,              0.0,               0.0,
						0.0,     focalLength * ratio,      0.0,               0.0,
						0.0,             0.0,         far * divides, -far * near * divides,
						0.0,             0.0,              1.0,               0.0,
				));
			}

			@vertex
			fn vs_main(in: VertexInput) -> VertexOutput {
				var out: VertexOutput;
				let ratio = 640.0 / 480.0;
				var offset = vec2f(0.0);

				// Scale the object
				let S = transpose(mat4x4f(
					0.3,  0.0, 0.0, 0.0,
					0.0,  0.3, 0.0, 0.0,
					0.0,  0.0, 0.3, 0.0,
					0.0,  0.0, 0.0, 1.0,
				));

				// Translate the object
				let T = transpose(mat4x4f(
					1.0,  0.0, 0.0, 0.5,
					0.0,  1.0, 0.0, 0.0,
					0.0,  0.0, 1.0, 0.0,
					0.0,  0.0, 0.0, 1.0,
				));

				// Rotate the model in the XY plane
				let angle1 = uMyUniforms.time;
				let c1 = cos(angle1);
				let s1 = sin(angle1);
				let R1 = transpose(mat4x4f(
					c1,  s1, 0.0, 0.0,
					-s1,  c1, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					0.0,  0.0, 0.0, 1.0,
				));

				// Tilt the view point in the YZ plane
				// by three 8th of turn (1 turn = 2 pi)
				let angle2 = 3.0 * pi / 4.0;
				let c2 = cos(angle2);
				let s2 = sin(angle2);
				let R2 = transpose(mat4x4f(
					1.0, 0.0, 0.0, 0.0,
					0.0,  c2,  s2, 0.0,
					0.0, -s2,  c2, 0.0,
					0.0,  0.0, 0.0, 1.0,
				));

				// Move the view point
				let focalPoint = vec3f(0.0, 0.0, -2.0);
				let T2 = transpose(mat4x4f(
					1.0,  0.0, 0.0, -focalPoint.x,
					0.0,  1.0, 0.0, -focalPoint.y,
					0.0,  0.0, 1.0, -focalPoint.z,
					0.0,  0.0, 0.0,     1.0,
				));

				// Compose and apply rotations
				// (S then T then R1 then R2, remember this reads backwards)
				let homogeneous_position = vec4f(in.position, 1.0);
				let viewspace_position = T2 * R2 * R1 * T * S * homogeneous_position;

				// Perspective projection
				let P = makePerspectiveProj(ratio, 0.01 /* near */, 100.0 /* far */, 2.0 /* focalLength */);
				out.position = P * viewspace_position;

				out.color = in.color;
				return out;
			}

			@fragment
			fn fs_main(in: VertexOutput) -> @location(0) vec4f {
				let color = in.color * uMyUniforms.color.rgb;
				// Gamma-correction
				let corrected_color = pow(color, vec3f(2.2));
				return vec4f(corrected_color, uMyUniforms.color.a);
			}
			)";
			m_renderer->SetShader(shaderSource);

			//
			std::vector<float> vertexData;
			std::vector<uint16_t> indexData;
			auto success = Geometry::load3DGeometry(RESOURCE_DIR "/webgpu.txt", vertexData, indexData, 3);
			if (!success) 
			{
				std::cerr << "Could not load geometry!" << std::endl;
				return;
			}
			//

			// Vertex fetch
			// We now have 2 attributes
			std::vector<wgpu::VertexAttribute> vertexAttribs(2);

			// Position attribute
			vertexAttribs[0].shaderLocation = 0;
			vertexAttribs[0].format = wgpu::VertexFormat::Float32x3;
			vertexAttribs[0].offset = 0;

			// Color attribute
			vertexAttribs[1].shaderLocation = 1;
			vertexAttribs[1].format = wgpu::VertexFormat::Float32x3; // different type!
			vertexAttribs[1].offset = 3 * sizeof(float); // non null offset!

			wgpu::VertexBufferLayout vertexBufferLayout;
			vertexBufferLayout.attributeCount = (uint32_t)vertexAttribs.size();
			vertexBufferLayout.attributes = vertexAttribs.data();
			// stride
			vertexBufferLayout.arrayStride = 6 * sizeof(float);
			vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;


			m_renderer->SetVertexBufferData(vertexData, vertexBufferLayout);
			m_renderer->SetIndexBufferData(indexData);

			// Create binding layout (don't forget to = Default)
			wgpu::BindGroupLayoutEntry bGLayoutEntry = wgpu::Default;
			// The binding index as used in the @binding attribute in the shader
			bGLayoutEntry.binding = 0;
			// The stage that needs to access this resource
			bGLayoutEntry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
			bGLayoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
			bGLayoutEntry.buffer.minBindingSize = sizeof(MyUniforms);
			// Make this binding dynamic so we can offset it between draw calls
			bGLayoutEntry.buffer.hasDynamicOffset = true;

			m_renderer->SetSizeOfUniform(sizeof(MyUniforms));
			m_renderer->SetBindGroupLayoutEntry(bGLayoutEntry);

			m_renderer->CreateUniformBuffer(1);

			m_renderer->Init();	
        }

		if (m_renderer)
		{
			m_renderer->BeginRenderPass();

			float time = static_cast<float>(glfwGetTime());

			// Upload first value

			// float angle1 = 2.0f;
			// constexpr float PI = 3.14159265358979323846f;
			// float angle2 = 3.0f * PI / 4.0f;
			// glm::vec3 focalPoint(0.0, 0.0, -2.0);

			// // Option B:
			// glm::mat4x4 S = glm::scale(glm::mat4x4(1.0), glm::vec3(0.3f));
			// glm::mat4x4 T1 = glm::translate(glm::mat4x4(1.0), glm::vec3(0.5, 0.0, 0.0));
			// glm::mat4x4 R1 = glm::rotate(glm::mat4x4(1.0), angle1, glm::vec3(0.0, 0.0, 1.0));
			// m_uniformData.modelMatrix = R1 * T1 * S;

			// glm::mat4x4 R2 = glm::rotate(glm::mat4x4(1.0), -angle2, glm::vec3(1.0, 0.0, 0.0));
			// glm::mat4x4 T2 = glm::translate(glm::mat4x4(1.0), -focalPoint);
			// m_uniformData.viewMatrix = T2 * R2;
			//

			// Option C:
			// glm::mat4x4 M(1.0);
			// M = glm::rotate(M, angle1, glm::vec3(0.0, 0.0, 1.0));
			// M = glm::translate(M, glm::vec3(0.5, 0.0, 0.0));
			// M = glm::scale(M, glm::vec3(0.3f));
			// m_uniformData.modelMatrix = M;

			// glm::mat4x4 V(1.0);
			// V = glm::translate(V, -focalPoint);
			// V = glm::rotate(V, -angle2, glm::vec3(1.0, 0.0, 0.0));
			// m_uniformData.viewMatrix = V;
			//
			
			// float focalLength = 2.0;
			// float fov = 2 * glm::atan(1 / focalLength);
			// float ratio = m_viewportWidth / m_viewportHeight;
			// float near = 0.01f;
			// float far = 100.0f;
			// m_uniformData.projectionMatrix = glm::perspective(fov, ratio, near, far);

			// angle1 = time;
			// R1 = glm::rotate(glm::mat4x4(1.0), angle1, glm::vec3(0.0, 0.0, 1.0));
			// m_uniformData.modelMatrix = R1 * T1 * S;

			m_uniformData.time = time * 0.95f; // glfwGetTime returns a double
			m_uniformData.color = { 0.0f, 1.0f, 0.4f, 1.0f };
			m_renderer->SetUniformBufferData(&m_uniformData, 0);
			////

			// Upload second value
			m_uniformData.time = time * 1.05f; // glfwGetTime returns a double
			m_uniformData.color = { 1.0f, 1.0f, 1.0f, 0.7f };
			m_renderer->SetUniformBufferData(&m_uniformData, 1);
			////                         				^^^^^^^^^^^^^ beware of the non-null offset!

			m_renderer->RenderIndexed(0);
			m_renderer->RenderIndexed(1);
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