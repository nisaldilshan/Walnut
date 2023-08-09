#pragma once

#include <memory>

namespace Walnut {

	class RenderingBackend
	{
	public:
		enum class BACKEND
		{
			None = 0, Vulkan = 1, OpenGL = 2
		};
	public:
		virtual ~RenderingBackend() = default;

		virtual void Init() = 0;
		virtual void Clear() = 0;

        virtual void SetupGraphicsAPI() = 0;

		static BACKEND GetBackend() { return s_backend; }
		static std::unique_ptr<RenderingBackend> Create();
	private:
		static BACKEND s_backend;
	};

}
