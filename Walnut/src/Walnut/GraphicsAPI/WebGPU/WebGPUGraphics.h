#pragma once

#include <webgpu/webgpu.hpp>

class GLFWwindow;

namespace GraphicsAPI
{

class WebGPU
{
public:
    static void CreateSurface(wgpu::Instance instance, GLFWwindow* window);
	static void CreateDevice(wgpu::Instance instance);
	static void FreeGraphicsResources();
    static wgpu::Surface GetSurface();
	static wgpu::Device GetDevice();
};

}