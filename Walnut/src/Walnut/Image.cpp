#include "Image.h"

#if (RENDERER_BACKEND == 1)
#include "GraphicsAPI/OpenGL/OpenGLImage.h"
#elif (RENDERER_BACKEND == 2)
#include "GraphicsAPI/Vulkan/VulkanImage.h"
#elif (RENDERER_BACKEND == 3)
#include "GraphicsAPI/WebGPU/WebGPUImage.h"
#else
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "ImageFormat.h"

namespace Walnut {

	namespace Utils {

		uint32_t BytesPerPixel(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGBA:    return 4;
				case ImageFormat::RGBA32F: return 16;
				case ImageFormat::None:	assert(false);
			}
			return 0;
		}
	}

#if (RENDERER_BACKEND == 1)
		static std::unique_ptr<GraphicsAPI::OpenGLImage> CreateBackendImage()
		{
			return std::make_unique<GraphicsAPI::OpenGLImage>();
		}
#elif (RENDERER_BACKEND == 2)
		static std::unique_ptr<GraphicsAPI::VulkanImage> CreateBackendImage()
		{
			return std::make_unique<GraphicsAPI::VulkanImage>();
		}
#elif (RENDERER_BACKEND == 3)
		static std::unique_ptr<GraphicsAPI::WebGPUImage> CreateBackendImage()
		{
			return std::make_unique<GraphicsAPI::WebGPUImage>();
		}
#else
#endif

	Image::Image(std::string_view path)
		: m_Filepath(path)
		, m_Width(0), m_Height(0), m_Format(ImageFormat::None)
		, m_rendererBackendImage(CreateBackendImage())
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
		
		AllocateMemory();
		SetData(data);
		stbi_image_free(data);
	}

	Image::Image(uint32_t width, uint32_t height, ImageFormat format)
		: m_Filepath()
		, m_Width(width), m_Height(height), m_Format(format)
		, m_rendererBackendImage(CreateBackendImage())
	{
		AllocateMemory();

		auto imageData = new uint8_t[m_Width * m_Height * Walnut::Utils::BytesPerPixel(format)];
		SetData(imageData);
		delete[] imageData;
	}

	Image::~Image()
	{
		Release();
	}

	void Image::AllocateMemory()
	{
		m_rendererBackendImage->CreateImage(m_Format, m_Width, m_Height);
		m_rendererBackendImage->CreateImageView();
		m_rendererBackendImage->CreateSampler();
		m_rendererBackendImage->CreateDescriptorSet();
	}

	void Image::Release()
	{
		m_rendererBackendImage->ResourceFree();
	}

	void Image::SetData(const void* data)
	{
		const size_t uploadSize = m_Width * m_Height * Utils::BytesPerPixel(m_Format);
		m_rendererBackendImage->UploadToBuffer(data, uploadSize);
	}

    uint64_t Image::GetHandle() // returns the image handle to be used in ImGui::Image()
    { 
		return m_rendererBackendImage->GetHandleForImGui(); 
	}

    void Image::Resize(uint32_t width, uint32_t height)
	{
		if (m_rendererBackendImage->ImageAvailable() && m_Width == width && m_Height == height)
			return;

		// TODO: max size?

		m_Width = width;
		m_Height = height;

		Release();
		AllocateMemory();
	}

}
