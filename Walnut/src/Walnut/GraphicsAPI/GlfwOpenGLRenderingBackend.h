#pragma once

#include "../RenderingBackend.h"

namespace Walnut {

	class GlfwOpenGLRenderingBackend : public RenderingBackend
	{
	public:
		virtual void Init(GLFWwindow* windowHandle) override
        {
            
        }

		void SetupGraphicsAPI() override
		{

		}

		GLFWwindow* GetWindowHandle() override
		{
			return nullptr;
		}

		virtual void Clear() override
        {
            
        }
	};


}