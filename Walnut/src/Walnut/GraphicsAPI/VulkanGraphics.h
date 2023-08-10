#pragma once

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

namespace Utils 
{
    static uint32_t GetVulkanMemoryType(VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties, uint32_t type_bits)
    {
        VkPhysicalDeviceMemoryProperties prop;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &prop);
        for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
        {
            if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
                return i;
        }
        
        return 0xffffffff;
    }
}


namespace GraphicsAPI
{

class Vulkan
{
public:
	static void SetupVulkan(const char** extensions, uint32_t extensions_count);
	static void AddWindowHandle(GLFWwindow* windowHandle);
	static void SetupVulkanWindow(int width, int height);
	static void CleanupVulkan();
	static void CleanupVulkanWindow();
	static void FrameRender(ImDrawData* draw_data);
	static void FramePresent();
	static void ConfigureRendererBackend(GLFWwindow* window);
	static void UploadFonts();
	static VkCommandBuffer GetCommandBuffer(bool begin);
	static void FlushCommandBuffer(VkCommandBuffer commandBuffer);
	static void ResizeVulkanWindow(int width, int height);
	static bool NeedSwapChainRebuild();
	static void SetSwapChainRebuildStatus(bool status);
	static void GraphicsDeviceWaitIdle();
	static void SetClearColor(ImVec4 clear_color);
	static void FreeGraphicsResources();

	// IMAGE

	static size_t CreateUploadBuffer(size_t upload_size);
	static void CreateImage(VkFormat vulkanFormat, uint32_t width, uint32_t height);
	static void CreateImageView(VkFormat vulkanFormat);
	static void CopyToImage(VkCommandBuffer command_buffer, uint32_t width, uint32_t height);
	static void UploadToBuffer(const void* data, size_t uploadSize, size_t alignedSize);
	static void CreateSampler();
	static void CreateDescriptorSet();
	static VkDescriptorSet GetDescriptorSet();
	static bool ImageAvailable();
	static void SubmitResourceFree(); // std::function<void()> &&func
	
};






























}
