#include "Image.h"

#ifdef USE_OPENGL_RENDERER
#include "GraphicsAPI/OpenGLImage.h"
#else
#include "GraphicsAPI/VulkanImage.h"
#endif


#include "Application.h"
#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Walnut {

	namespace Utils {

		static uint32_t BytesPerPixel(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGBA:    return 4;
				case ImageFormat::RGBA32F: return 16;
				case ImageFormat::None:	assert(false);
			}
			return 0;
		}
		
		static VkFormat WalnutFormatToVulkanFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGBA:    return VK_FORMAT_R8G8B8A8_UNORM;
				case ImageFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
				case ImageFormat::None:	assert(false);
			}
			return (VkFormat)0;
		}

#ifdef USE_OPENGL_RENDERER
		static std::unique_ptr<GraphicsAPI::OpenGLImage> CreateBackendImage()
		{
			return std::make_unique<GraphicsAPI::OpenGLImage>();
		}
#else
		static std::unique_ptr<GraphicsAPI::VulkanImage> CreateBackendImage()
		{
			return std::make_unique<GraphicsAPI::VulkanImage>();
		}
#endif 

	}

	Image::Image(std::string_view path)
		: m_Filepath(path)
		, m_rendererBackendImage(Utils::CreateBackendImage())
	{
		int width, height, channels;
		uint8_t* data = nullptr;

		if (stbi_is_hdr(m_Filepath.c_str()))
		{
			data = (uint8_t*)stbi_loadf(m_Filepath.c_str(), &width, &height, &channels, 4);
			m_Format = ImageFormat::RGBA32F;
		}
		else
		{
			data = stbi_load(m_Filepath.c_str(), &width, &height, &channels, 4);
			m_Format = ImageFormat::RGBA;
		}

		m_Width = width;
		m_Height = height;
		
		AllocateMemory(m_Width * m_Height * Utils::BytesPerPixel(m_Format));
		SetData(data);
		stbi_image_free(data);
	}

	Image::Image(uint32_t width, uint32_t height, ImageFormat format, const void* data)
		: m_Width(width), m_Height(height), m_Format(format)
		, m_rendererBackendImage(Utils::CreateBackendImage())
	{
		AllocateMemory(m_Width * m_Height * Utils::BytesPerPixel(m_Format));
		if (data)
			SetData(data);
	}

	Image::~Image()
	{
		Release();
	}

	void Image::AllocateMemory(uint64_t size)
	{
		VkFormat vulkanFormat = Utils::WalnutFormatToVulkanFormat(m_Format);

		// Create the Image
		{
			m_rendererBackendImage->CreateImage(vulkanFormat, m_Width, m_Height);
		}

		// Create the Image View:
		{
			m_rendererBackendImage->CreateImageView(vulkanFormat);
		}

		// Create sampler:
		{
			m_rendererBackendImage->CreateSampler();
		}

		// Create the Descriptor Set:
		m_rendererBackendImage->CreateDescriptorSet();
	}

	void Image::Release()
	{
		m_rendererBackendImage->ResourceFree();
	}

	void Image::SetData(const void* data)
	{
		size_t upload_size = m_Width * m_Height * Utils::BytesPerPixel(m_Format);

		VkResult err;

		if (!m_rendererBackendImage->GetStagingBuffer())
		{
			// Create the Upload Buffer
			m_AlignedSize = m_rendererBackendImage->CreateUploadBuffer(upload_size);
		}

		// Upload to Buffer
		{
			m_rendererBackendImage->UploadToBuffer(data, upload_size, m_AlignedSize);
		}
	}

    void* Image::GetDescriptorSet() // originally returned the type VkDescriptorSet
    { 
		return m_rendererBackendImage->GetDescriptorSet(); 
	}

    void Image::Resize(uint32_t width, uint32_t height)
	{
		if (m_rendererBackendImage->ImageAvailable() && m_Width == width && m_Height == height)
			return;

		// TODO: max size?

		m_Width = width;
		m_Height = height;

		Release();
		AllocateMemory(m_Width * m_Height * Utils::BytesPerPixel(m_Format));
	}

}
