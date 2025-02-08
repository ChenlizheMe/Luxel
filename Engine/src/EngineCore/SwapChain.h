#pragma once

#include "Core.h"

#include "Device.h"

namespace Luxel
{
	struct SwapChainSupportProperties
	{
		VkSurfaceCapabilitiesKHR capabiliies;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class LUXEL_API SwapChain
	{
	public:
		SwapChain(GLFWwindow* const window, Device* const d);
		~SwapChain();
		SwapChain(const SwapChain&) = delete;
		void operator=(const SwapChain&) = delete;

		ui32 currentFrame;
		ui32 swapChainImageCount;

		VkExtent2D swapChainExtent;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		VkRenderPass GetRenderPass();
		VkResult AccaquireNextImage(ui32* imageIndex);
		VkResult SubmitCommandBuffers(VkCommandBuffer* commandBuffer, ui32* imageIndex);

	private:
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDepthResources();
		void CreateFrameBuffers();
		void CreateSyncObjects();
		
		SwapChainSupportProperties QuerySwapChainSupport();

		VkSurfaceFormatKHR ChooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkFormat FindDepthFormat();

		GLFWwindow* const window;
		Device* const device;

		VkSwapchainKHR instance;
		VkFormat swapChainFormat;
		VkPresentModeKHR swapChainPresentMode;

		VkRenderPass renderPass;

		std::vector<VkImage> colorImages;
		std::vector<VkImageView> colorImageViews;

		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImagesMemory;
		std::vector<VkImageView> depthImageViews;

		// semaphores
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
	};
}