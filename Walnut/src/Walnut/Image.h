#pragma once

#include <string>
#include <memory>

#include "ImageFormat.h"
#include "ExportConfig.h"

namespace GraphicsAPI
{
#if (RENDERER_BACKEND == 1)
class OpenGLImage;
typedef OpenGLImage ImageType;
#elif (RENDERER_BACKEND == 2)
class VulkanImage;
typedef VulkanImage ImageType;
#elif (RENDERER_BACKEND == 3)
class WebGPUImage;
typedef WebGPUImage ImageType;
#else
#endif
}

namespace Walnut {

	namespace Utils
	{
		uint32_t BytesPerPixel(ImageFormat format);
	} // namespace Utils
	

	class Walnut_API Image
	{
	public:
		Image(std::string_view path);
		Image(uint32_t width, uint32_t height, ImageFormat format);
		~Image();

		void SetData(const void* data);

		uint64_t GetHandle();

		void Resize(uint32_t width, uint32_t height);

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		std::unique_ptr<GraphicsAPI::ImageType>& PlatformImageRef() { return m_rendererBackendImage; }
	private:
		void AllocateMemory();
		void Release();

		std::string m_Filepath;
		uint32_t m_Width, m_Height;
		ImageFormat m_Format;
		std::unique_ptr<GraphicsAPI::ImageType> m_rendererBackendImage;
		size_t m_AlignedSize = 0;
	};

}



