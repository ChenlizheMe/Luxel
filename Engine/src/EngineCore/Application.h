#pragma once

#include "pch.h"

#include "Core.h"

#include "log.h"
#include "RenderPipeline.h"
#include "Device.h"
#include "SwapChain.h"

namespace Luxel
{
	class LUXEL_API Application
	{
	public:
		Application();
		virtual ~Application();
		Application(const Application&) = delete;
		void operator=(const Application&) = delete;

		void Init(int width, int height, const char* name);
		void Run();

	private:
		void CreatePipelineLayout();
		void CreateCommandBuffers();
		void DrawFrame();

		GLFWwindow* window;
		RenderPipeline* renderPipeline;
		SwapChain* swapChain;
		Device* device;
		
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffer;
	};

	Application* CreateApplication();
}