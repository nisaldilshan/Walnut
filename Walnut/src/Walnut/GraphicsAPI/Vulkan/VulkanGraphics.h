#pragma once

#include <functional>
#include <optional>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

namespace GraphicsAPI
{

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

struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

class Vulkan
{
public:
	static void check_vk_result(VkResult err);
	static void SetupVulkan(const char** extensions, uint32_t extensions_count);
	static void SetupVulkanWindow(int width, int height);
	static void CleanupVulkan();
	static void CleanupVulkanWindow();
	static void FrameRender(void* draw_data);
	static void FramePresent();
	static void ConfigureRendererBackend();
	static void UploadFonts();
	static VkCommandPool GetCommandPool();
	static void QueueSubmit(VkSubmitInfo info);
	static void ResizeVulkanWindow(int width, int height);
	static bool NeedSwapChainRebuild();
	static void SetSwapChainRebuildStatus(bool status);
	static void GraphicsDeviceWaitIdle();
	static void FreeGraphicsResources();
	static void SubmitResourceFree(std::function<void()> func);
	static void SetClearColor(ImVec4 clear_color);
	static QueueFamilyIndices FindQueueFamilies();
	
	static VkInstance GetInstance();
	static VkDevice GetDevice();
	static VkPhysicalDevice GetPhysicalDevice();
	static uint32_t GetQueueFamilyIndex();
	static VkQueue GetDeviceQueue();
	static VkAllocationCallbacks* GetAllocator();
	static VkSurfaceKHR* GetSurface();
};






























}
