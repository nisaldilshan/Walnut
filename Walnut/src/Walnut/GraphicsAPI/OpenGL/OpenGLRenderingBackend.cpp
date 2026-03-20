#include "OpenGLRenderingBackend.h"

#include "OpenGLGraphics.h"

#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <Walnut/Image.h>

#include <iostream>

namespace Walnut
{ 
    GLuint CompileShader(GLenum type, const std::string_view source) 
	{
		GLuint id = glCreateShader(type);
		const char* src = source.data();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		int result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);
		if (result == GL_FALSE) 
		{
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			std::vector<char> message(length);
			glGetShaderInfoLog(id, length, &length, message.data());
			
			std::cerr << "Failed to compile " 
					<< (type == GL_VERTEX_SHADER ? "vertex" : "fragment") 
					<< " shader!" << std::endl;
			std::cerr << message.data() << std::endl;
			
			glDeleteShader(id);
			return 0;
		}

		return id;
	}

	GLuint CreateShaderProgram(const std::string_view vertexShaderSrc, const std::string_view fragmentShaderSrc) 
	{
		GLuint program = glCreateProgram();
		GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
		GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

		// Attach and link them together
		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);

		// Error handling for linking
		int result;
		glGetProgramiv(program, GL_LINK_STATUS, &result);
		if (result == GL_FALSE) 
		{
			int length;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			std::vector<char> message(length);
			glGetProgramInfoLog(program, length, &length, message.data());
			
			std::cerr << "Failed to link shader program!" << std::endl;
			std::cerr << message.data() << std::endl;
			
			glDeleteProgram(program);
			return 0;
		}

		// Always detach and delete the individual shaders once linked
		// They are now baked into the program object and just taking up memory
		glDetachShader(program, vs);
		glDetachShader(program, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);

		return program;
	}

	OpenGLRenderingBackend::OpenGLRenderingBackend()
	{}

	OpenGLRenderingBackend::~OpenGLRenderingBackend()
	{}

    GLuint g_emptyVAO;
	void OpenGLRenderingBackend::CreateMainImagePipeline(std::unique_ptr<Image>& mainImage)
    {
		glGenVertexArrays(1, &g_emptyVAO);

		constexpr std::string_view vertexSrc = R"(
			#version 330 core
			out vec2 v_TexCoord;
			void main() {
				v_TexCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
				gl_Position = vec4(v_TexCoord * 2.0 - 1.0, 0.0, 1.0);
			}
		)";

		constexpr std::string_view fragmentSrc = R"(
			#version 330 core
			layout(location = 0) out vec4 color;
			in vec2 v_TexCoord;
			uniform float u_TilingFactor;
			uniform sampler2D u_Texture;
			void main() {
				color = texture(u_Texture, v_TexCoord * u_TilingFactor);
			}
		)";

		m_imageRenderShaderProgram = CreateShaderProgram(vertexSrc, fragmentSrc);

		if (m_imageRenderShaderProgram == 0) {
			assert(false && "Shader program creation failed!");
		}

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
    }

    void OpenGLRenderingBackend::DestroyMainImagePipeline()
    {
    }

    void OpenGLRenderingBackend::FrameRender(std::unique_ptr<Image> &mainImage)
    {
		// Bind your texture to texture unit 0
		glActiveTexture(GL_TEXTURE0);
		const auto textureiD = mainImage->GetHandle();
		glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)textureiD);

		// Bind your shader and textures via uniforms
		glUseProgram(m_imageRenderShaderProgram);
		glUniform1i(glGetUniformLocation(m_imageRenderShaderProgram, "u_Texture"), 0);
		glUniform1f(glGetUniformLocation(m_imageRenderShaderProgram, "u_TilingFactor"), 1.0f); 


		glBindVertexArray(g_emptyVAO); // Bind the empty VAO
		// Draw exactly 3 vertices. The Vertex Shader handles the rest!
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0); // Unbind the VAO
	}

	void OpenGLRenderingBackend::FrameRenderImGui(void* draw_data)
	{
		glDisable(GL_FRAMEBUFFER_SRGB); // <--- DISABLE THIS for ImGui
		ImGui_ImplOpenGL3_RenderDrawData((ImDrawData*)draw_data);
	}

} // namespace Walnut