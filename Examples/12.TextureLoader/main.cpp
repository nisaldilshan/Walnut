#define DAWN_DEBUG_BREAK_ON_ERROR 1

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include <Walnut/Timer.h>

#include "../Common/Renderer3D.h"
#include "../Common/Geometry.h"
#include "../Common/Texture.h"
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
	glm::vec2 uv;
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

class Renderer2DLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		bool success = Geometry::loadGeometryFromObjWithUV<VertexAttributes>(RESOURCE_DIR "/Meshes/fourareen.obj", m_vertexData);
		if (!success) 
		{
			std::cerr << "Could not load geometry!" << std::endl;
			assert(false);
		}

		m_texHandle = Texture::loadTexture(RESOURCE_DIR "/Textures/fourareen2K_albedo.jpg");
		assert(m_texHandle && m_texHandle->GetWidth() > 0 && m_texHandle->GetHeight() > 0 && m_texHandle->GetMipLevelCount() > 0);

		m_shaderSource = R"(
		struct VertexInput {
			@location(0) position: vec3f,
			@location(1) normal: vec3f,
			@location(2) color: vec3f,
			@location(3) uv: vec2f,
		};

		struct VertexOutput {
			@builtin(position) position: vec4f,
			@location(0) color: vec3f,
			@location(1) normal: vec3f,
			@location(2) uv: vec2f,
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

		@group(0) @binding(0) var<uniform> uMyUniforms: MyUniforms;
		@group(0) @binding(1) var gradientTexture: texture_2d<f32>;
		@group(0) @binding(2) var textureSampler: sampler;

		@vertex
		fn vs_main(in: VertexInput) -> VertexOutput {
			var out: VertexOutput;
			out.position = uMyUniforms.projectionMatrix * uMyUniforms.viewMatrix * uMyUniforms.modelMatrix * vec4f(in.position, 1.0);
			// Forward the normal
			out.normal = (uMyUniforms.modelMatrix * vec4f(in.normal, 0.0)).xyz;
			out.color = in.color;
			out.uv = in.uv;

			return out;
		}

		@fragment
		fn fs_main(in: VertexOutput) -> @location(0) vec4f {
			// We remap UV coords to actual texel coordinates
			let color = textureSample(gradientTexture, textureSampler, in.uv).rgb;

			// Gamma-correction
			let corrected_color = pow(color, vec3f(2.2));
			return vec4f(corrected_color, uMyUniforms.color.a);
		}
		)";

		m_renderer.reset();
		m_renderer = std::make_unique<Renderer3D>();

		assert(m_shaderSource);
		m_renderer->SetShader(m_shaderSource);

		m_renderer->CreateTextureSampler();
		m_renderer->CreateTexture(m_texHandle->GetWidth(), m_texHandle->GetHeight(), m_texHandle->GetData(), m_texHandle->GetMipLevelCount());
	}

	virtual void OnDetach() override
	{}

	virtual void OnUpdate(float ts) override
	{
        Walnut::Timer timer;
		if (m_viewportWidth == 0 || m_viewportHeight == 0)
			return;

        if (!m_renderer ||
            m_viewportWidth != m_renderer->GetWidth() ||
            m_viewportHeight != m_renderer->GetHeight())
        {
			InitialCode();
        }

		if (m_renderer)
		{
			m_renderer->BeginRenderPass();
			const float time = static_cast<float>(glfwGetTime());
			constexpr float PI = 3.14159265358979323846f;		
			m_uniformData.viewMatrix = glm::lookAt(glm::vec3(-2.0f, -3.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0, 0, 1));
			m_uniformData.projectionMatrix = glm::perspective(30 * (PI / 180), (float)(m_viewportWidth / m_viewportHeight), 0.01f, 100.0f);

			// Upload first value
			float angle1 = 2.0f;
			glm::mat4x4 M1(1.0);
			angle1 = time * 0.9f;
			M1 = glm::rotate(M1, angle1, glm::vec3(0.0, 0.0, 1.0));
			M1 = glm::translate(M1, glm::vec3(0.0, 0.0, 0.0));
			M1 = glm::scale(M1, glm::vec3(0.3f));
			m_uniformData.modelMatrix = M1;

			m_uniformData.time = time; // glfwGetTime returns a double
			m_uniformData.color = { 0.0f, 1.0f, 0.4f, 1.0f };
			m_renderer->SetUniformBufferData(&m_uniformData, 0);
			////


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
	void InitialCode()
	{
		m_renderer->OnResize(m_viewportWidth, m_viewportHeight);

		std::vector<wgpu::VertexAttribute> vertexAttribs(4);

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
		vertexAttribs[2].format = wgpu::VertexFormat::Float32x3;
		vertexAttribs[2].offset = offsetof(VertexAttributes, color);

		// UV attribute
		vertexAttribs[3].shaderLocation = 3;
		vertexAttribs[3].format = wgpu::VertexFormat::Float32x2;
		vertexAttribs[3].offset = offsetof(VertexAttributes, uv);

		wgpu::VertexBufferLayout vertexBufferLayout;
		vertexBufferLayout.attributeCount = (uint32_t)vertexAttribs.size();
		vertexBufferLayout.attributes = vertexAttribs.data();
		// stride
		vertexBufferLayout.arrayStride = sizeof(VertexAttributes);
		vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

		assert(m_vertexData.size() > 0);
		m_renderer->SetVertexBufferData(m_vertexData.data(), m_vertexData.size() * sizeof(VertexAttributes), vertexBufferLayout);

		// Create binding layouts

		// Since we now have 2 bindings, we use a vector to store them
		std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(3, wgpu::Default);
		// The uniform buffer binding that we already had
		wgpu::BindGroupLayoutEntry& uniformBindingLayout = bindingLayoutEntries[0];
		uniformBindingLayout.binding = 0;
		uniformBindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
		uniformBindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
		uniformBindingLayout.buffer.minBindingSize = sizeof(MyUniforms);
		uniformBindingLayout.buffer.hasDynamicOffset = true;

		// The texture binding
		wgpu::BindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries[1];
		textureBindingLayout.binding = 1;
		textureBindingLayout.visibility = wgpu::ShaderStage::Fragment;
		textureBindingLayout.texture.sampleType = wgpu::TextureSampleType::Float;
		textureBindingLayout.texture.viewDimension = wgpu::TextureViewDimension::_2D;

		// The sampler binding
		wgpu::BindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries[2];
		samplerBindingLayout.binding = 2;
		samplerBindingLayout.visibility = wgpu::ShaderStage::Fragment;
		samplerBindingLayout.sampler.type = wgpu::SamplerBindingType::Filtering;

		m_renderer->SetSizeOfUniform(sizeof(MyUniforms));
		m_renderer->SetBindGroupLayoutEntries(bindingLayoutEntries);

		m_renderer->CreateUniformBuffer(1, Uniform::UniformType::ModelViewProjection);

		m_renderer->Init();
	}

    std::unique_ptr<Renderer3D> m_renderer;
    uint32_t m_viewportWidth = 0;
    uint32_t m_viewportHeight = 0;
    float m_lastRenderTime = 0.0f;

	MyUniforms m_uniformData;
	std::vector<VertexAttributes> m_vertexData;
	std::unique_ptr<Texture::TextureHandle> m_texHandle = nullptr;
	const char* m_shaderSource = nullptr;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Renderer3D Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<Renderer2DLayer>();
	return app;
}