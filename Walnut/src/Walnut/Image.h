#pragma once

#include <string>

namespace GraphicsAPI
{
#ifdef USE_OPENGL_RENDERER
	class OpenGLImage;
#else
	class VulkanImage;
#endif
}

namespace Walnut {

	enum class ImageFormat
	{
		None = 0,
		RGBA,
		RGBA32F
	};

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
	private:
		std::string m_Filepath;
		uint32_t m_Width = 0, m_Height = 0;
		ImageFormat m_Format = ImageFormat::None;
#ifdef USE_OPENGL_RENDERER
		std::unique_ptr<GraphicsAPI::OpenGLImage> m_rendererBackendImage;
#else
		std::unique_ptr<GraphicsAPI::VulkanImage> m_rendererBackendImage;
#endif
		size_t m_AlignedSize = 0;
	};

}



