#include "OpenGLRenderingBackend.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "OpenGLGraphics.h"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <glad/glad.h>
#include <iostream>

#include <Walnut/Image.h>

namespace Walnut
{
	GLuint CompileShader(GLenum type, const std::string& source) 
	{
		GLuint id = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		// Error handling
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

	GLuint CreateShaderProgram(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc) 
	{
		GLuint program = glCreateProgram();
		
		// Compile both shaders
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

	void OpenGLRenderingBackend::Init(GLFWwindow *windowHandle)
	{
		m_windowHandle = windowHandle;
		
		glfwMakeContextCurrent(m_windowHandle);
		GraphicsAPI::OpenGL::SetupOpenGL((void*)glfwGetProcAddress);
		
	}

	void OpenGLRenderingBackend::SetupWindow(int width, int height)
	{
		GraphicsAPI::OpenGL::SetupViewport(width, height);
	}

	bool OpenGLRenderingBackend::NeedToResizeWindow()
	{
		return false;
	}

	void OpenGLRenderingBackend::ResizeWindow(int width, int height)
	{
	}

	void OpenGLRenderingBackend::CreateImGuiPipeline()
	{
		ImGui_ImplGlfw_InitForOpenGL(m_windowHandle, true);
#ifdef __EMSCRIPTEN__
		ImGui_ImplOpenGL3_Init("#version 300 es");
#else
		ImGui_ImplOpenGL3_Init("#version 410");
#endif
	}

	void OpenGLRenderingBackend::StartImGuiFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	void OpenGLRenderingBackend::DestroyImGuiPipeline()
    {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
    }

	GLuint g_emptyVAO;
	GLuint g_shaderProgram;
	void OpenGLRenderingBackend::CreateMainImagePipeline(std::unique_ptr<Image>& mainImage)
    {
        //auto& platformImage = mainImage->PlatformImageRef();
		glGenVertexArrays(1, &g_emptyVAO);

		std::string vertexSrc = R"(
			#version 330 core
			out vec2 v_TexCoord;
			void main() {
				v_TexCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
				gl_Position = vec4(v_TexCoord * 2.0 - 1.0, 0.0, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core
			layout(location = 0) out vec4 color;
			in vec2 v_TexCoord;
			uniform float u_TilingFactor;
			uniform sampler2D u_Texture;
			void main() {
				color = texture(u_Texture, v_TexCoord * u_TilingFactor);
			}
		)";

		g_shaderProgram = CreateShaderProgram(vertexSrc, fragmentSrc);

		if (g_shaderProgram == 0) {
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

    void OpenGLRenderingBackend::FrameBegin()
    {
    }

    void OpenGLRenderingBackend::FrameRender(std::unique_ptr<Image> &mainImage)
    {
		// Bind your texture to texture unit 0
		glActiveTexture(GL_TEXTURE0);
		const auto textureiD = mainImage->GetDescriptorSet();
		glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)textureiD);

		// 2. Bind your shader and textures
		glUseProgram(g_shaderProgram);

		// Set your uniforms
		glUniform1i(glGetUniformLocation(g_shaderProgram, "u_Texture"), 0);
		glUniform1f(glGetUniformLocation(g_shaderProgram, "u_TilingFactor"), 1.0f); 

		// 3. Draw the full-screen triangle
		glBindVertexArray(g_emptyVAO); // Bind the empty VAO

		// Draw exactly 3 vertices. The Vertex Shader handles the rest!
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0); // Unbind
	}

	void OpenGLRenderingBackend::FrameRenderImGui(void* draw_data)
	{
		glDisable(GL_FRAMEBUFFER_SRGB); // <--- DISABLE THIS for ImGui
		ImGui_ImplOpenGL3_RenderDrawData((ImDrawData*)draw_data);
	}

    void OpenGLRenderingBackend::FrameEnd()
    {
    }

    void OpenGLRenderingBackend::FramePresent()
	{
		glfwSwapBuffers(m_windowHandle);
	}

	WalnutWindowHandleType* OpenGLRenderingBackend::GetWindowHandle()
	{
		return m_windowHandle;
	}

	void OpenGLRenderingBackend::Shutdown()
	{
		
	}

	void OpenGLRenderingBackend::Cleanup()
	{
	}

}