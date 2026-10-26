#include "VulkanGraphics.h"

#include <imgui_impl_vulkan.h>

#include <iostream>
#include <vector>
#include <array>
#include <functional>
#include <cstring>

namespace GraphicsAPI
{

void Vulkan::check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

static VkAllocationCallbacks* 	g_Allocator = NULL;
static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkQueue                  g_Queue = VK_NULL_HANDLE;
static VkDebugUtilsMessengerEXT g_DebugMessenger = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

static VkSurfaceKHR             g_surface = VK_NULL_HANDLE;
// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static ImGui_ImplVulkanH_Window g_MainWindowData;

static uint32_t                 g_MinImageCount = 2;
static bool                     g_SwapChainRebuild = false;
static VkImageUsageFlags        g_SwapChainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

// Unlike g_MainWindowData.FrameIndex, this is not the the swapchain image index
// and is always guaranteed to increase (eg. 0, 1, 2, 0, 1, 2)
static uint32_t s_CurrentFrameIndex = 0;

static std::vector<std::vector<std::function<void()>>> s_ResourceFreeQueue;

template<typename T> // either std::vector or ImVector
static bool IsExtensionAvailable(const T& properties, const char* extension)
{
    for (const VkExtensionProperties& p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

static bool IsLayerAvailable(const char* layerName)
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());
    for (const auto& layer : layers)
        if (strcmp(layer.layerName, layerName) == 0)
            return true;
    return false;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                          VkDebugUtilsMessageTypeFlagsEXT,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                          void*)
{
    fprintf(stderr, "[vulkan validation] %s\n", callbackData->pMessage);
    return VK_FALSE;
}

static void printAvailableDeviceExtensions(VkPhysicalDevice physicalDevice) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

    // 4. Print them out
    std::cout << "Available Device Extensions (" << extensionCount << "):" << std::endl;
    for (const auto& extension : extensions) {
        std::cout << "\t" << extension.extensionName 
                  << " (v" << extension.specVersion << ")" << std::endl;
    }
}

void Vulkan::CreateDevice(std::vector<const char*> extensions, const VulkanConfig& config)
{
	VkResult err;

	// Enumerate available extensions
	uint32_t properties_count;
	ImVector<VkExtensionProperties> properties;
	vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
	properties.resize(properties_count);
	err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);
	check_vk_result(err);

	// Enable required extensions
	VkInstanceCreateFlags instanceFlags = 0;
	if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	}
	if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
		extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		instanceFlags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}

	// Validation is best-effort: only turned on when asked for AND the layer/extension are
	// actually installed, so a machine without LunarG's SDK (or Conan's package) just gets none.
	std::vector<const char*> layers;
	bool useValidation = false;
	if (config.EnableValidation) {
		if (IsLayerAvailable("VK_LAYER_KHRONOS_validation")) {
			layers.push_back("VK_LAYER_KHRONOS_validation");
			useValidation = IsExtensionAvailable(properties, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			if (useValidation)
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			else
				fprintf(stderr, "[vulkan] VK_EXT_debug_utils is not available; validation messages will not be reported\n");
		} else {
			fprintf(stderr, "[vulkan] validation requested but VK_LAYER_KHRONOS_validation is not installed\n");
		}
	}

	VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
	messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
								  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
							  | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
							  | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messengerInfo.pfnUserCallback = DebugUtilsCallback;

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Walnut App";
	appInfo.pEngineName = "Walnut";
	appInfo.apiVersion = VK_API_VERSION_1_2;

	// Create Vulkan Instance
	{
		VkInstanceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pNext = useValidation ? &messengerInfo : nullptr; // also reports errors from vkCreateInstance itself
		create_info.flags = instanceFlags;
		create_info.pApplicationInfo = &appInfo;
		create_info.enabledLayerCount = (uint32_t)layers.size();
		create_info.ppEnabledLayerNames = layers.data();
		create_info.enabledExtensionCount = (uint32_t)extensions.size();
		create_info.ppEnabledExtensionNames = extensions.data();

		err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
		check_vk_result(err);

		if (useValidation) {
			auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugUtilsMessengerEXT");
			IM_ASSERT(vkCreateDebugUtilsMessengerEXT != NULL);
			err = vkCreateDebugUtilsMessengerEXT(g_Instance, &messengerInfo, g_Allocator, &g_DebugMessenger);
			check_vk_result(err);
		}
	}

	// Select GPU
	{
		uint32_t gpu_count;
		err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, NULL);
		check_vk_result(err);
		IM_ASSERT(gpu_count > 0);

		VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
		err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus);
		check_vk_result(err);

		// If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
		// most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
		// dedicated GPUs) is out of scope of this sample.
		int use_gpu = 0;
		for (int i = 0; i < (int)gpu_count; i++)
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(gpus[i], &properties);
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				use_gpu = i;
				break;
			}
		}

		// Print driver details
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(gpus[use_gpu], &properties);
			std::cout << "Graphics Info:" << std::endl;
			std::cout << "\tdeviceName: " << properties.deviceName << std::endl;
			std::cout << "\tapiVersion: " << VK_API_VERSION_MAJOR(properties.apiVersion) << "." 
											<< VK_API_VERSION_MINOR(properties.apiVersion) << "." 
											<< VK_API_VERSION_PATCH(properties.apiVersion) << std::endl;
			std::cout << "\tdriverVersion: " << properties.driverVersion << std::endl;
		}


		g_PhysicalDevice = gpus[use_gpu];
		free(gpus);
	}

	// Select graphics queue family
	{
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
		VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
		vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
		for (uint32_t i = 0; i < count; i++)
			if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				g_QueueFamily = i;
				break;
			}
		free(queues);
		IM_ASSERT(g_QueueFamily != (uint32_t)-1);
	}

	// Create Logical Device (with 1 queue)
	{
		uint32_t availableExtensionsCount = 0;
    	vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &availableExtensionsCount, nullptr);
    	std::vector<VkExtensionProperties> availableExtensions(availableExtensionsCount);
    	vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &availableExtensionsCount, availableExtensions.data());

		constexpr std::array<const char*, 2> deviceExtensions = {
			"VK_KHR_swapchain",
			"VK_KHR_maintenance1" //VK_KHR_MAINTENANCE_1_EXTENSION_NAME
		};
		for (const auto &extension : deviceExtensions)
		{
			assert(IsExtensionAvailable(availableExtensions, extension));
		}

		const float queue_priority[] = { 1.0f };
		VkDeviceQueueCreateInfo queue_info[1] = {};
		queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info[0].queueFamilyIndex = g_QueueFamily;
		queue_info[0].queueCount = 1;
		queue_info[0].pQueuePriorities = queue_priority;
		VkDeviceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
		create_info.pQueueCreateInfos = queue_info;
		create_info.enabledExtensionCount = deviceExtensions.size();
		create_info.ppEnabledExtensionNames = deviceExtensions.data();
		err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
		check_vk_result(err);
		vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
	}

	// Create Descriptor Pool
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
		check_vk_result(err);
	}
}

void Vulkan::SetupVulkanWindow(int width, int height)
{
	g_MainWindowData.Surface = g_surface;

	// Check for WSI support
	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, g_MainWindowData.Surface, &res);
	if (res != VK_TRUE)
	{
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	// Select Surface Format
	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	g_MainWindowData.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, g_MainWindowData.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
	g_MainWindowData.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, g_MainWindowData.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));

	// Frames can only be read back (ReadSwapChainImage) when the surface allows copying from its images.
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_PhysicalDevice, g_MainWindowData.Surface, &surfaceCapabilities);
	g_SwapChainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	IM_ASSERT(g_MinImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device,
										&g_MainWindowData, g_QueueFamily, g_Allocator,
										width, height, g_MinImageCount, g_SwapChainImageUsage);
    s_ResourceFreeQueue.resize(g_MainWindowData.ImageCount);
	
	// set clear color
	ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	g_MainWindowData.ClearValue.color.float32[0] = clearColor.x * clearColor.w;
    g_MainWindowData.ClearValue.color.float32[1] = clearColor.y * clearColor.w;
    g_MainWindowData.ClearValue.color.float32[2] = clearColor.z * clearColor.w;
    g_MainWindowData.ClearValue.color.float32[3] = clearColor.w;
}

void Vulkan::CleanupVulkan()
{
	vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

	if (g_DebugMessenger != VK_NULL_HANDLE) {
		auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessengerEXT(g_Instance, g_DebugMessenger, g_Allocator);
	}

	vkDestroyDevice(g_Device, g_Allocator);
	vkDestroyInstance(g_Instance, g_Allocator);
}

void Vulkan::CleanupVulkanWindow()
{
	ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
}

void Vulkan::FrameBegin()
{
	VkResult err;

	VkSemaphore image_acquired_semaphore = g_MainWindowData.FrameSemaphores[g_MainWindowData.SemaphoreIndex].ImageAcquiredSemaphore;
	err = vkAcquireNextImageKHR(g_Device, g_MainWindowData.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &g_MainWindowData.FrameIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		g_SwapChainRebuild = true;
		return;
	}
	check_vk_result(err);

	s_CurrentFrameIndex = (s_CurrentFrameIndex + 1) % g_MainWindowData.ImageCount;

	ImGui_ImplVulkanH_Frame* fd = &g_MainWindowData.Frames[g_MainWindowData.FrameIndex];
	{
		err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
		check_vk_result(err);

		err = vkResetFences(g_Device, 1, &fd->Fence);
		check_vk_result(err);
	}
	
	{
		// Free resources in queue
		for (auto& func : s_ResourceFreeQueue[s_CurrentFrameIndex])
			func();
		s_ResourceFreeQueue[s_CurrentFrameIndex].clear();
	}
	{
		err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
		check_vk_result(err);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
		check_vk_result(err);
	}
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = g_MainWindowData.RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = g_MainWindowData.Width;
		info.renderArea.extent.height = g_MainWindowData.Height;
		info.clearValueCount = 1;
		info.pClearValues = &g_MainWindowData.ClearValue;
		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}
}

void Vulkan::FrameEnd()
{
	VkResult err;
	VkSemaphore image_acquired_semaphore = g_MainWindowData.FrameSemaphores[g_MainWindowData.SemaphoreIndex].ImageAcquiredSemaphore;
	VkSemaphore render_complete_semaphore = g_MainWindowData.FrameSemaphores[g_MainWindowData.SemaphoreIndex].RenderCompleteSemaphore;
	ImGui_ImplVulkanH_Frame* fd = &g_MainWindowData.Frames[g_MainWindowData.FrameIndex];
	// Submit command buffer
	vkCmdEndRenderPass(fd->CommandBuffer);

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &image_acquired_semaphore;
	submitInfo.pWaitDstStageMask = &wait_stage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &fd->CommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &render_complete_semaphore;

	err = vkEndCommandBuffer(fd->CommandBuffer);
	check_vk_result(err);
	err = vkQueueSubmit(g_Queue, 1, &submitInfo, fd->Fence);
	check_vk_result(err);
}

bool Vulkan::ReadSwapChainImage(std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height)
{
	const VkFormat format = g_MainWindowData.SurfaceFormat.format;
	const bool isBGRA = format == VK_FORMAT_B8G8R8A8_UNORM || format == VK_FORMAT_B8G8R8A8_SRGB;
	const bool isRGBA = format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_R8G8B8A8_SRGB;
	if (g_SwapChainRebuild || !(g_SwapChainImageUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) || !(isBGRA || isRGBA))
		return false;

	width = g_MainWindowData.Width;
	height = g_MainWindowData.Height;
	const VkDeviceSize size = VkDeviceSize(width) * height * 4;
	ImGui_ImplVulkanH_Frame* fd = &g_MainWindowData.Frames[g_MainWindowData.FrameIndex];

	// Host-visible buffer the swapchain image is copied into
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkBuffer buffer;
	VkResult err = vkCreateBuffer(g_Device, &bufferInfo, g_Allocator, &buffer);
	check_vk_result(err);
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(g_Device, buffer, &req);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = req.size;
	allocInfo.memoryTypeIndex = Utils::GetVulkanMemoryType(g_PhysicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, req.memoryTypeBits);
	VkDeviceMemory memory;
	err = vkAllocateMemory(g_Device, &allocInfo, g_Allocator, &memory);
	check_vk_result(err);
	err = vkBindBufferMemory(g_Device, buffer, memory, 0);
	check_vk_result(err);

	VkCommandBufferAllocateInfo cmdAllocInfo = {};
	cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocInfo.commandPool = fd->CommandPool;
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdAllocInfo.commandBufferCount = 1;
	VkCommandBuffer cmd;
	err = vkAllocateCommandBuffers(g_Device, &cmdAllocInfo, &cmd);
	check_vk_result(err);
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	err = vkBeginCommandBuffer(cmd, &beginInfo);
	check_vk_result(err);

	// The render pass left the image ready to present: make it a copy source, then hand it back to presentation.
	VkImageMemoryBarrier toTransfer = {};
	toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	toTransfer.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	toTransfer.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toTransfer.image = fd->Backbuffer;
	toTransfer.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	// ALL_COMMANDS also covers the render pass's final layout transition to PRESENT_SRC.
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
						 0, 0, nullptr, 0, nullptr, 1, &toTransfer);

	VkBufferImageCopy region = {};
	region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.imageExtent = { width, height, 1 };
	vkCmdCopyImageToBuffer(cmd, fd->Backbuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);

	VkImageMemoryBarrier toPresent = toTransfer;
	toPresent.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	toPresent.dstAccessMask = 0;
	toPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkBufferMemoryBarrier toHost = {};
	toHost.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	toHost.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	toHost.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
	toHost.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toHost.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	toHost.buffer = buffer;
	toHost.size = VK_WHOLE_SIZE;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						 0, 0, nullptr, 1, &toHost, 1, &toPresent);

	err = vkEndCommandBuffer(cmd);
	check_vk_result(err);
	// Runs after the frame's own submission (same queue) and waits until the copy is done.
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	QueueSubmit(submitInfo);
	vkFreeCommandBuffers(g_Device, fd->CommandPool, 1, &cmd);

	void* mapped = nullptr;
	err = vkMapMemory(g_Device, memory, 0, size, 0, &mapped);
	check_vk_result(err);
	pixels.resize(size_t(size));
	memcpy(pixels.data(), mapped, pixels.size());
	vkUnmapMemory(g_Device, memory);
	vkDestroyBuffer(g_Device, buffer, g_Allocator);
	vkFreeMemory(g_Device, memory, g_Allocator);

	if (isBGRA)
		for (size_t i = 0; i < pixels.size(); i += 4)
			std::swap(pixels[i], pixels[i + 2]);
	return true;
}

void Vulkan::FramePresent()
{
	if (g_SwapChainRebuild)
		return;
	VkSemaphore render_complete_semaphore = g_MainWindowData.FrameSemaphores[g_MainWindowData.SemaphoreIndex].RenderCompleteSemaphore;
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &render_complete_semaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &g_MainWindowData.Swapchain;
	info.pImageIndices = &g_MainWindowData.FrameIndex;
	VkResult err = vkQueuePresentKHR(g_Queue, &info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
	{
		g_SwapChainRebuild = true;
		return;
	}
	check_vk_result(err);
	g_MainWindowData.SemaphoreIndex = (g_MainWindowData.SemaphoreIndex + 1) % g_MainWindowData.ImageCount; // Now we can use the next set of semaphores
}

VkCommandPool Vulkan::GetCommandPool()
{
    VkCommandPool command_pool = g_MainWindowData.Frames[g_MainWindowData.FrameIndex].CommandPool;
	return command_pool;
}

void Vulkan::QueueSubmit(VkSubmitInfo info)
{
    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VkFence fence;
    auto err = vkCreateFence(g_Device, &fenceCreateInfo, nullptr, &fence);
    check_vk_result(err);

    err = vkQueueSubmit(g_Queue, 1, &info, fence);
    check_vk_result(err);

	constexpr uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;
    err = vkWaitForFences(g_Device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
    check_vk_result(err);

    vkDestroyFence(g_Device, fence, nullptr);
}

void Vulkan::ResizeVulkanWindow(int width, int height)
{
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device,
										&g_MainWindowData, g_QueueFamily, g_Allocator,
										width, height, g_MinImageCount, g_SwapChainImageUsage);
    g_MainWindowData.FrameIndex = 0;
}

bool Vulkan::NeedSwapChainRebuild()
{
    return g_SwapChainRebuild;
}

void Vulkan::SetSwapChainRebuildStatus(bool status)
{
    g_SwapChainRebuild = status;
}

void Vulkan::GraphicsDeviceWaitIdle()
{
    // Cleanup
    VkResult err = vkDeviceWaitIdle(g_Device);
    check_vk_result(err);
}

void Vulkan::FreeGraphicsResources()
{
    // Free resources in queue
    for (auto& queue : s_ResourceFreeQueue)
    {
        for (auto& func : queue)
            func();
    }
    s_ResourceFreeQueue.clear();
}

void Vulkan::SubmitResourceFree(std::function<void()> func)
{
	s_ResourceFreeQueue[s_CurrentFrameIndex].emplace_back(func);
}

QueueFamilyIndices Vulkan::FindQueueFamilies() 
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

VkInstance Vulkan::GetInstance()
{
    return g_Instance;
}

VkDevice Vulkan::GetDevice()
{
	return g_Device;
}

VkPhysicalDevice Vulkan::GetPhysicalDevice()
{
	return g_PhysicalDevice;
}

uint32_t Vulkan::GetQueueFamily()
{
    return g_QueueFamily;
}

VkQueue Vulkan::GetQueue()
{
    return g_Queue;
}

VkAllocationCallbacks* Vulkan::GetAllocator()
{
    return g_Allocator;
}

VkSurfaceKHR* Vulkan::GetSurface()
{
    return &g_surface;
}

VkDescriptorPool Vulkan::GetDescriptorPool()
{
	return g_DescriptorPool;
}

uint32_t Vulkan::GetMinImageCount()
{
    return g_MinImageCount;
}

uint32_t Vulkan::GetImageCount()
{
    return g_MainWindowData.ImageCount;
}

VkFormat Vulkan::GetDepthFormat()
{
	return VK_FORMAT_D32_SFLOAT;
}

VkCommandBuffer Vulkan::GetCurrentCommmandBuffer()
{
	const ImGui_ImplVulkanH_Frame* fd = &g_MainWindowData.Frames[g_MainWindowData.FrameIndex];
    return fd->CommandBuffer;
}

VkRenderPass Vulkan::GetRenderPass()
{
    return g_MainWindowData.RenderPass;
}

uint32_t Vulkan::GetWidth()
{
    return g_MainWindowData.Width;
}

uint32_t Vulkan::GetHeight()
{
    return g_MainWindowData.Height;
};

// IMAGE




















} // namespace