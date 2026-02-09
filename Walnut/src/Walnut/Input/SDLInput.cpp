#include "Input.h"

#include <Walnut/Application.h>
#include <SDL3/SDL.h>

namespace Walnut {

	bool Input::IsKeyDown(KeyCode keycode)
	{
		WalnutWindowHandleType* windowHandle = Application::Get().GetWindowHandle();
		int state;// = glfwGetKey(windowHandle, (int)keycode);
		return false; //state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonDown(MouseButton button)
	{
		WalnutWindowHandleType* windowHandle = Application::Get().GetWindowHandle();
		int state;// = glfwGetMouseButton(windowHandle, (int)button);
		return false; //state == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePosition()
	{
		WalnutWindowHandleType* windowHandle = Application::Get().GetWindowHandle();

		float x, y;
		SDL_GetMouseState(&x, &y);
		return { x, y };
	}

	void Input::SetCursorMode(CursorMode mode)
	{
		// GLFWwindow* windowHandle = Application::Get().GetWindowHandle();
		// glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}

}