#pragma once

#include <string>
#include <memory>

#include "ImageFormat.h"

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

	class Image
	{
	public:
		Image(std::string_view path);
		Image(uint32_t width, uint32_t height, ImageFormat format, const void* data = nullptr);
		~Image();

		void SetData(const void* data);

		void* GetDescriptorSet(); // originally returned the type VkDescriptorSet

		void Resize(uint32_t width, uint32_t height);

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
	private:
		void AllocateMemory(uint64_t size);
		void Release();

		std::string m_Filepath;
		uint32_t m_Width = 0, m_Height = 0;
		ImageFormat m_Format = ImageFormat::None;
		std::unique_ptr<GraphicsAPI::ImageType> m_rendererBackendImage;
		size_t m_AlignedSize = 0;
	};

}



