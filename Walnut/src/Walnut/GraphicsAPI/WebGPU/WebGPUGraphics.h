#pragma once

#include <imgui_impl_glfw.h>
#include <webgpu/webgpu.hpp>

class GLFWwindow;

namespace GraphicsAPI
{

class WebGPU
{
public:
	static void CreateInstance(wgpu::InstanceDescriptor desc);
    static void CreateSurface(GLFWwindow* window);
	static void CreateDevice();
	static void FreeGraphicsResources();

	static wgpu::Instance GetInstance();
    static wgpu::Surface GetSurface();
	static wgpu::TextureFormat GetSwapChainFormat();
	static wgpu::Device GetDevice();
	static wgpu::Queue GetQueue();
};

}