#pragma once

#include "KeyCodes.h"

namespace Walnut {

	class Input
	{
	public:
		static bool IsKeyDown(KeyCode keycode);
		static bool IsMouseButtonDown(MouseButton button);

		static std::array<float, 2> GetMousePosition();

		static void SetCursorMode(CursorMode mode);
	};

}
