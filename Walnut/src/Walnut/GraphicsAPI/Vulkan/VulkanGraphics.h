#pragma once

#include <functional>
#include <optional>
#include <vector>
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

// Config the host (e.g. RenderSys) hands in, instead of Vulkan setup being hard-coded.
struct VulkanConfig
{
	// Enables VK_LAYER_KHRONOS_validation and VK_EXT_debug_utils when the layer is installed;
	// silently has no effect otherwise. Messages are printed to stderr.
	bool EnableValidation = false;
};

class Vulkan
{
public:
	static void check_vk_result(VkResult err);
	static void CreateDevice(std::vector<const char*> extensions, const VulkanConfig& config);
	static void SetupVulkanWindow(int width, int height);
	static void CleanupVulkan();
	static void CleanupVulkanWindow();
	static void FrameBegin();
	static void FrameEnd();
	// Copies the swapchain image FrameEnd submitted into pixels (RGBA8, top row first). Call before FramePresent.
	static bool ReadSwapChainImage(std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height);
	static void FramePresent();
	static VkCommandPool GetCommandPool();
	static void QueueSubmit(VkSubmitInfo info);
	static void ResizeVulkanWindow(int width, int height);
	// Destroys the window's swapchain and surface, then sets the window up again with the surface that createSurface
	// makes. Returns false, leaving the window without a surface and IsSurfaceLost() true, when createSurface fails,
	// e.g. on Android while the app is in the background and has no native window.
	static bool RecreateVulkanWindow(const std::function<bool(VkSurfaceKHR*)>& createSurface, int width, int height);
	// The surface can no longer be presented to, e.g. on Android after its native window was destroyed. Frames are
	// skipped until RecreateVulkanWindow succeeds.
	static bool IsSurfaceLost();
	static void SetSurfaceLost();
	// Whether the current frame acquired a swapchain image, so that it can be recorded and presented
	static bool IsFrameActive();
	// True when the swapchain has to be rebuilt or the surface recreated
	static bool NeedSwapChainRebuild();
	static void SetSwapChainRebuildStatus(bool status);
	static void GraphicsDeviceWaitIdle();
	static void FreeGraphicsResources();
	static void SubmitResourceFree(std::function<void()> func);
	static QueueFamilyIndices FindQueueFamilies();
	
	static VkInstance GetInstance();
	static VkDevice GetDevice();
	static VkPhysicalDevice GetPhysicalDevice();
	static uint32_t GetQueueFamily();
	static VkQueue GetQueue();
	static VkAllocationCallbacks* GetAllocator();
	static VkSurfaceKHR* GetSurface();
	static VkDescriptorPool GetDescriptorPool();
	static uint32_t GetMinImageCount();
	static uint32_t GetImageCount();
	static VkFormat GetDepthFormat();
	static VkCommandBuffer GetCurrentCommmandBuffer();
	static VkRenderPass GetRenderPass();
	static uint32_t GetWidth();
	static uint32_t GetHeight();
};






























}
