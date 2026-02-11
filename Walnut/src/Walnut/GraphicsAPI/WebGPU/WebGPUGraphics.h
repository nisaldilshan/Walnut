#pragma once

#include <webgpu/webgpu.hpp>
#include "../../WindowHandle.h"

namespace GraphicsAPI
{

class WebGPU
{
public:
	static void CreateInstance(wgpu::InstanceDescriptor desc);
    static void CreateSurface(WalnutWindowHandleType* window);
	static void CreateDevice();
	static void FreeGraphicsResources();

	static wgpu::Instance GetInstance();
    static wgpu::Surface GetSurface();
	static wgpu::TextureFormat GetSwapChainFormat();
	static wgpu::Device GetDevice();
	static wgpu::Queue GetQueue();
};

}