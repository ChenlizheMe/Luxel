#include "pch.h"

#include "SwapChain.h"

namespace Luxel
{
	SwapChain::SwapChain(GLFWwindow* w, Device* const d) :window{ w }, device{ d }
	{
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDepthResources();
		CreateFrameBuffers();
		CreateSyncObjects();
	}

	SwapChain::~SwapChain()
	{
		vkDestroySwapchainKHR(device->GetDevice(), instance, nullptr);
	}

	void SwapChain::CreateSwapChain()
	{
		Info("Create Swap chain instance.");
		SwapChainSupportProperties properties = QuerySwapChainSupport();

		VkSurfaceFormatKHR format = ChooseSwapChainFormat(properties.formats);
		VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(properties.presentModes);
		VkExtent2D extent = ChooseSwapExtent(properties.capabiliies);

		ui32 imageCount = properties.capabiliies.minImageCount + 1;
		if (properties.capabiliies.maxImageCount != 0 && imageCount > properties.capabiliies.maxImageCount) {
			imageCount = properties.capabiliies.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapChainCreateInfo{};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = device->GetSurface();

		swapChainCreateInfo.minImageCount = imageCount;
		swapChainCreateInfo.imageFormat = format.format;
		swapChainCreateInfo.imageColorSpace = format.colorSpace;
		swapChainCreateInfo.imageArrayLayers = 1;
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapChainCreateInfo.imageExtent = extent;

		QueueFamilyIndices queueFamilyIndices = device->GetQueueFamilyIndices();
		ui32 indices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };

		if (indices[0] != indices[1]) {
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = indices;
		}
		else {
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapChainCreateInfo.queueFamilyIndexCount = 0;
			swapChainCreateInfo.pQueueFamilyIndices = nullptr;
		}

		swapChainCreateInfo.preTransform = properties.capabiliies.currentTransform;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainCreateInfo.presentMode = presentMode;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device->GetDevice(), &swapChainCreateInfo, nullptr, &instance) != VK_SUCCESS) {
			Error("Failed to create swap chain.");
			throw std::runtime_error("Failed to create swap chain.");
		}

		swapChainFormat = format.format;
		swapChainExtent = extent;
	}

	void SwapChain::CreateImageViews()
	{
		Info("Create swap chain image views.");
		ui32 imageCount = 0;
		vkGetSwapchainImagesKHR(device->GetDevice(), instance, &imageCount, nullptr);
		imageViews.resize(imageCount);
		vkGetSwapchainImagesKHR(device->GetDevice(), instance, &imageCount, imageViews.data());
	}

	void SwapChain::CreateRenderPass()
	{
	}

	void SwapChain::CreateDepthResources()
	{
	}

	void SwapChain::CreateFrameBuffers()
	{
	}

	void SwapChain::CreateSyncObjects()
	{
	}

	SwapChainSupportProperties SwapChain::QuerySwapChainSupport()
	{
		SwapChainSupportProperties properties{};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->GetPhysicalDevice(), device->GetSurface(), &properties.capabiliies);

		ui32 formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDevice(), device->GetSurface(), &formatCount, nullptr);
		if (formatCount != 0) {
			properties.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDevice(), device->GetSurface(), &formatCount, properties.formats.data());
		}

		ui32 presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetPhysicalDevice(), device->GetSurface(), &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			properties.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetPhysicalDevice(), device->GetSurface(), &presentModeCount, properties.presentModes.data());
		}

		return properties;
	}

	VkSurfaceFormatKHR SwapChain::ChooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& formats) 
	{
		for (const auto& format : formats) {
			if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return format;
			}
		}

		Warning("Failed to find suitable swap chain format, use default swap chain format.");
		return formats[0];
		// throw std::runtime_error("Failed to find suitable swap chain format.");
	}

	VkPresentModeKHR SwapChain::ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes) 
	{
		for (const auto& mode : presentModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return mode;
			}
		}

		Warning("Failed to use VK_PRESENT_MODE_MAILBOX mode, use fifo mode to present images");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<ui32>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D swapChainExtent{
				static_cast<ui32>(width),
				static_cast<ui32>(height)
			};

			swapChainExtent.width = std::clamp(swapChainExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			swapChainExtent.height = std::clamp(swapChainExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return swapChainExtent;
		}
	}
}