#pragma once

namespace Walnut {

	enum class ImageFormat
	{
		None = 0,
		RGBA,
		RGBA32F
	};

namespace Utils
{
    typedef int VkFormat;
    VkFormat WalnutFormatToVulkanFormat(ImageFormat format);
} // namespace Utils
    
}