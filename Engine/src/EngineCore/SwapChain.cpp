#include "pch.h"

#include "SwapChain.h"

namespace Luxel
{
	SwapChain::SwapChain(GLFWwindow* w, Device* const d) :window{ w }, device{ d }
	{
		currentFrame = 0;

		CreateSwapChain();
		CreateImageViews();
		CreateDepthResources();
		CreateRenderPass();
		CreateFrameBuffers();
		CreateSyncObjects();
	}

	SwapChain::~SwapChain()
	{
		Info("Destroy swap chain.");


		if (instance != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(device->GetDevice(), instance, nullptr);
		}

		for (int i = 0; i < colorImageViews.size(); i++) {
			vkDestroyImageView(device->GetDevice(), colorImageViews[i], nullptr);
			vkDestroyImageView(device->GetDevice(), depthImageViews[i], nullptr);
			vkFreeMemory(device->GetDevice(), depthImagesMemory[i], nullptr);
		}

		for (int i = 0;i < depthImages.size();i++) {
			if (depthImages[i] != VK_NULL_HANDLE) {
				vkDestroyImage(device->GetDevice(), depthImages[i], nullptr);
			}
		}

		for (int i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(device->GetDevice(), swapChainFramebuffers[i], nullptr);
		}

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device->GetDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device->GetDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device->GetDevice(), inFlightFences[i], nullptr);
		}


		vkDestroyRenderPass(device->GetDevice(), renderPass, nullptr);
	}


	VkRenderPass SwapChain::GetRenderPass()
	{
		return renderPass;
	}

	VkResult SwapChain::AccaquireNextImage(ui32* imageIndex)
	{
		vkWaitForFences(device->GetDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		VkResult result = vkAcquireNextImageKHR(device->GetDevice(), instance, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);
		return result;
	}

	VkResult SwapChain::SubmitCommandBuffers(VkCommandBuffer* commandBuffer, ui32* imageIndex)
	{
		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(device->GetDevice(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}

		imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffer;

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device->GetDevice(), 1, &inFlightFences[currentFrame]);
		if (vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			Error("Failed to submit draw command buffer.");
			throw std::runtime_error("Failed to submit draw command buffer.");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { instance };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = imageIndex;
		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(device->GetPresentQueue(), &presentInfo);
		if (result != VK_SUCCESS) {
			Error("Failed to present swap chain image.");
			throw std::runtime_error("Failed to present swap chain image.");
		}

		return result;
	}

	void SwapChain::CreateSwapChain()
	{
		Info("Create Swap chain instance.");
		SwapChainSupportProperties properties = QuerySwapChainSupport();

		VkSurfaceFormatKHR format = ChooseSwapChainFormat(properties.formats);
		VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(properties.presentModes);
		VkExtent2D extent = ChooseSwapExtent(properties.capabiliies);

		ui32 prepareImageCount = properties.capabiliies.minImageCount + 1;
		if (properties.capabiliies.maxImageCount != 0 && prepareImageCount > properties.capabiliies.maxImageCount) {
			prepareImageCount = properties.capabiliies.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapChainCreateInfo{};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = device->GetSurface();

		swapChainCreateInfo.minImageCount = prepareImageCount;
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
		swapChainPresentMode = presentMode;

		swapChainImageCount = 0;
		vkGetSwapchainImagesKHR(device->GetDevice(), instance, &swapChainImageCount, nullptr);
		colorImages.resize(swapChainImageCount);
		vkGetSwapchainImagesKHR(device->GetDevice(), instance, &swapChainImageCount, colorImages.data());
	}

	void SwapChain::CreateImageViews()
	{
		Info("Create swap chain image views.");
		colorImageViews.resize(swapChainImageCount);
		
		for (int i = 0;i < swapChainImageCount;i++) {
			VkImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.image = colorImages[i];
			imageViewCreateInfo.format = swapChainFormat;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;
			
			if (vkCreateImageView(device->GetDevice(), &imageViewCreateInfo, nullptr, &colorImageViews[i]) != VK_SUCCESS) {
				Error("Failed to create image view.");
				throw std::runtime_error("Failed to create image view.");
			}
		}

	}

	void SwapChain::CreateRenderPass()
	{
		Info("Create render pass.");
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subPassDescription{};
		subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subPassDescription.colorAttachmentCount = 1;
		subPassDescription.pColorAttachments = &colorAttachmentRef;
		subPassDescription.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency subPassDependency{};
		subPassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subPassDependency.dstSubpass = 0;
		subPassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subPassDependency.srcAccessMask = 0;
		subPassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 2;
		renderPassCreateInfo.pAttachments = attachments;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subPassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &subPassDependency;

		if (vkCreateRenderPass(device->GetDevice(), &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
			Error("Failed to create render pass.");
			throw std::runtime_error("Failed to create render pass.");
		}
	}

	void SwapChain::CreateDepthResources()
	{
		Info("Create depth resources.");
		VkFormat depthFormat = FindDepthFormat();

		depthImages.resize(swapChainImageCount);
		depthImagesMemory.resize(swapChainImageCount);
		depthImageViews.resize(swapChainImageCount);

		for (int i = 0;i < swapChainImageCount;i++) {
			VkImageCreateInfo imageCreateInfo{};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.extent.width = swapChainExtent.width;
			imageCreateInfo.extent.height = swapChainExtent.height;
			imageCreateInfo.extent.depth = 1;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.format = depthFormat;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.flags = 0;

			if (vkCreateImage(device->GetDevice(), &imageCreateInfo, nullptr, &depthImages[i]) != VK_SUCCESS) {
				Error("Failed to create depth image.");
				throw std::runtime_error("Failed to create depth image.");
			}

			VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(device->GetDevice(), depthImages[i], &memoryRequirements);
			VkMemoryAllocateInfo memoryAllocateInfo{};
			memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocateInfo.allocationSize = memoryRequirements.size;
			memoryAllocateInfo.memoryTypeIndex = device->FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			if (vkAllocateMemory(device->GetDevice(), &memoryAllocateInfo, nullptr, &depthImagesMemory[i]) != VK_SUCCESS) {
				Error("Failed to allocate memory for depth image.");
				throw std::runtime_error("Failed to allocate memory for depth image.");
			}

			if (vkBindImageMemory(device->GetDevice(), depthImages[i], depthImagesMemory[i], 0) != VK_SUCCESS) {
				Error("Failed to bind memory for depth image.");
				throw std::runtime_error("Failed to bind memory for depth image.");
			}

			VkImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.image = depthImages[i];
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = depthFormat;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device->GetDevice(), &imageViewCreateInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS) {
				Error("Failed to create depth image view.");
				throw std::runtime_error("Failed to create depth image view.");
			}
		}
	}

	void SwapChain::CreateFrameBuffers()
	{
		Info("Create frame buffers.");
		swapChainFramebuffers.resize(swapChainImageCount);

		for (int i = 0;i < swapChainImageCount;i++) {
			VkImageView attachments[] = {
				colorImageViews[i],
				depthImageViews[i]
			};
			VkFramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.renderPass = renderPass;
			framebufferCreateInfo.attachmentCount = 2;
			framebufferCreateInfo.pAttachments = attachments;
			framebufferCreateInfo.width = swapChainExtent.width;
			framebufferCreateInfo.height = swapChainExtent.height;
			framebufferCreateInfo.layers = 1;
			if (vkCreateFramebuffer(device->GetDevice(), &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				Error("Failed to create framebuffer.");
				throw std::runtime_error("Failed to create framebuffer.");
			}
		}
	}

	void SwapChain::CreateSyncObjects()
	{
		Info("Create sync objects.");
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		imagesInFlight.resize(swapChainImageCount, VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0;i < MAX_FRAMES_IN_FLIGHT;i++) {
			if (vkCreateSemaphore(device->GetDevice(), &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device->GetDevice(), &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device->GetDevice(), &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				Error("Failed to create sync objects.");
				throw std::runtime_error("Failed to create sync objects.");
			}
		}
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

	VkFormat SwapChain::FindDepthFormat() {
		return device->FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}