#include "pch.h"

#include "Application.h"

namespace Luxel
{
	Application::Application()
	{

	}

	Application::~Application()
	{
		// wait for device to be idle
		vkQueueWaitIdle(device->GetGraphicsQueue());
		vkQueueWaitIdle(device->GetPresentQueue());

		// destory pipeline layout
		vkDestroyPipelineLayout(device->GetDevice(), pipelineLayout, nullptr);

		// destory command buffers
		vkFreeCommandBuffers(device->GetDevice(), device->GetCommandPool(), static_cast<ui32>(commandBuffer.size()), commandBuffer.data());

		// destory swap chain
		delete swapChain;

		// destory render pipeline
		delete renderPipeline;

		// destory device
		delete device;

		// destory glfw window
		Info("Destory GLFW Window and Terminate.");
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Application::Init(int width, int height, const char* name)
	{
		// query for vulkan extensions.
		Debug("Query for vulkan extensions");
		ui32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		Debug(extensionCount, "extensions supported.");
		std::vector<VkExtensionProperties> avaliableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, avaliableExtensions.data());
		for (const auto& extension : avaliableExtensions) {
			Debug("avaliable extension:", extension.extensionName);
		}

		// glfw init.
		Info("Init GLFW window.");
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		Info("Window Setting [", "Width:", width, "Height:", height, "]");
		window = glfwCreateWindow(width, height, name, nullptr, nullptr);

		// setup devices
		Info("Setup device.");
		//device = new Device(name, true, enableExtensions, window);
		device = new Device{ name, true, window };

		// create swap chain
		Info("Create swap chain.");
		swapChain = new SwapChain(window, device);

		// create pipeline layout
		Info("Create pipeline layout.");
		CreatePipelineLayout();

		// create render pipeline
		Info("Create render pipeline.");
		renderPipeline = new RenderPipeline{
			device,
			swapChain,
			"./shaders/simple_shader.vert.spv",
			"./shaders/simple_shader.frag.spv",
			RenderPipeline::DefaultPipelineConfigInfo(pipelineLayout, width, height, swapChain->GetRenderPass())
		};

		Info("Create command buffers.");
		CreateCommandBuffers();
	}

	void Application::Run()
	{
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			DrawFrame();
		}
	}

	void Application::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 0;
		pipelineLayoutCreateInfo.pSetLayouts = nullptr;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(device->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			Error("Failed to create pipeline layout.");
			throw std::runtime_error("Failed to create pipeline layout.");
		}
	}

	void Application::CreateCommandBuffers()
	{
		commandBuffer.resize(swapChain->swapChainImageCount);
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = device->GetCommandPool();
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = static_cast<ui32>(commandBuffer.size());

		if (vkAllocateCommandBuffers(device->GetDevice(), &commandBufferAllocateInfo, commandBuffer.data()) != VK_SUCCESS) {
			Error("Failed to allocate command buffers.");
			throw std::runtime_error("Failed to allocate command buffers.");
		}

		for (int i = 0;i < commandBuffer.size();i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffer[i], &beginInfo) != VK_SUCCESS) {
				Error("Failed to begin recording command buffer.");
				throw std::runtime_error("Failed to begin recording command buffer.");
			}

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = swapChain->GetRenderPass();
			renderPassBeginInfo.framebuffer = swapChain->swapChainFramebuffers[i];
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = swapChain->swapChainExtent;

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassBeginInfo.clearValueCount = static_cast<ui32>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			renderPipeline->Bind(commandBuffer[i]);
			vkCmdDraw(commandBuffer[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(commandBuffer[i]);
			if (vkEndCommandBuffer(commandBuffer[i]) != VK_SUCCESS) {
				Error("Failed to record command buffer.");
				throw std::runtime_error("Failed to record command buffer.");
			}
		}
	}
	void Application::DrawFrame()
	{
		ui32 imageIndex;
		VkResult result = swapChain->AccaquireNextImage(&imageIndex);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			Error("Failed to acquire next image.");
			throw std::runtime_error("Failed to acquire next image.");
		}

		result = swapChain->SubmitCommandBuffers(&commandBuffer[imageIndex], &imageIndex);
	}
}