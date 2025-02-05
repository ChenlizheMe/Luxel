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

		GLFWwindow* const window;
		Device* const device;

		VkSwapchainKHR instance;
		VkFormat swapChainFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkImage> imageViews;
	};
}