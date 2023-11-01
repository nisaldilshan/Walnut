#include <Walnut/Application.h>

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class SandboxLayer : public Walnut::Layer
{
public:
	SandboxLayer();
	virtual void OnUpdate(float ts) override;
	virtual void OnUIRender() override;
	//virtual void OnEvent(Hazel::Event& e) override;

private:
	// Hazel::ShaderLibrary m_ShaderLibrary;
	// std::shared_ptr<Hazel::Shader> m_Shader;
	// std::shared_ptr<Hazel::VertexArray> m_VertexArray;

	// std::shared_ptr<Hazel::Shader> m_FlatColorShader;
	// std::shared_ptr<Hazel::VertexArray> m_SquareVA;

	// std::shared_ptr<Hazel::Texture2D> m_Texture;
	// std::shared_ptr<Hazel::Texture2D> m_ChernoLogoTexture;

	//OrthographicCameraController m_CameraController;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };

};
