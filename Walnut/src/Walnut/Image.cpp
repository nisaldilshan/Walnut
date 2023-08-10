#include "Image.h"

#include "imgui.h"
#include <imgui_impl_vulkan.h>

#include "Application.h"
#include "GraphicsAPI/VulkanGraphics.h"

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

	}

	Image::Image(std::string_view path)
		: m_Filepath(path)
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
			GraphicsAPI::Vulkan::CreateImage(vulkanFormat, m_Width, m_Height);
		}

		// Create the Image View:
		{
			GraphicsAPI::Vulkan::CreateImageView(vulkanFormat);
		}

		// Create sampler:
		{
			GraphicsAPI::Vulkan::CreateSampler();
		}

		// Create the Descriptor Set:
		GraphicsAPI::Vulkan::CreateDescriptorSet();
	}

	void Image::Release()
	{
		Application::SubmitGraphicsResourceFree();

		
	}

	void Image::SetData(const void* data)
	{
		size_t upload_size = m_Width * m_Height * Utils::BytesPerPixel(m_Format);

		VkResult err;

		if (!m_AlignedSize)
		{
			// Create the Upload Buffer
			m_AlignedSize = GraphicsAPI::Vulkan::CreateUploadBuffer(upload_size);
		}

		// Upload to Buffer
		{
			GraphicsAPI::Vulkan::UploadToBuffer(data, upload_size, m_AlignedSize);
		}


		// Copy to Image
		{
			VkCommandBuffer command_buffer = Application::GetGraphicsCommandBuffer(true);

			GraphicsAPI::Vulkan::CopyToImage(command_buffer, m_Width, m_Height);

			Application::FlushGraphicsCommandBuffer(command_buffer);
		}
	}

    VkDescriptorSet Image::GetDescriptorSet() const
    { 
		return GraphicsAPI::Vulkan::GetDescriptorSet(); 
	}

    void Image::Resize(uint32_t width, uint32_t height)
	{
		if (GraphicsAPI::Vulkan::ImageAvailable() && m_Width == width && m_Height == height)
			return;

		// TODO: max size?

		m_Width = width;
		m_Height = height;

		Release();
		AllocateMemory(m_Width * m_Height * Utils::BytesPerPixel(m_Format));
	}

}
