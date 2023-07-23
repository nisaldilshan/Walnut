#pragma once

#include <string>

#include <vulkan/vulkan.h>

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

		VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }

		void Resize(uint32_t width, uint32_t height);

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
	private:
		void AllocateMemory(uint64_t size);
		void Release();
	private:
		uint32_t m_Width = 0, m_Height = 0;

		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		VkSampler m_Sampler;

		ImageFormat m_Format = ImageFormat::None;

		VkBuffer m_StagingBuffer;
		VkDeviceMemory m_StagingBufferMemory;

		size_t m_AlignedSize = 0;

		VkDescriptorSet m_DescriptorSet;

		std::string m_Filepath;
	};

}



