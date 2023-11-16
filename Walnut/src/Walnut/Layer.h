#pragma once

#include "ExportConfig.h"

namespace Walnut {

	class Walnut_API Layer
	{
	public:
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUpdate(float ts) {}
		virtual void OnUIRender() {}
	};

}