#pragma once

#include "pch.h"

#include "Core.h"

#include "log.h"

namespace Luxel
{
	struct QueueFamilyIndices {
		std::optional<ui32> graphicsFamily;
		std::optional<ui32> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class LUXEL_API Device
	{
	public:
		Device(const char* appName, bool enableValidation, GLFWwindow* window);
		~Device();
		Device(const Device&) = delete;
		void operator=(const Device&) = delete;

		VkDevice GetDevice();
		VkPhysicalDevice GetPhysicalDevice();
		VkSurfaceKHR GetSurface();
		QueueFamilyIndices GetQueueFamilyIndices();

	private:
		void CreateInstance(const char* appName);
		void SetupDebugMessenger();
		void CreateSurface(GLFWwindow* window);
		void PickupPhysicaclDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);

		VkResult CreateDebugUtilsMessengerEXT(
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
			const VkAllocationCallbacks* pAllocator, 
			VkDebugUtilsMessengerEXT* pDebugMessenger
		);

		void DestroyDebugUtilsMessengerEXT(
			VkInstance instance, 
			VkDebugUtilsMessengerEXT debugMessenger, 
			const VkAllocationCallbacks* pAllocator
		);

		void PopulateDebugUtilsMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		std::string GetQueueType(const ui32 queueFlag) const;

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, bool outputLog);
		void GetDeviceQueueCreateInfo(VkDeviceQueueCreateInfo& queueCreateInfo, const ui32 queueFamilyIndex, const ui32 queueCount, const float* priority);

		bool CheckValidationLayerSupport();
		bool CheckPhysicalDevice(const VkPhysicalDevice& device);

		VkInstance instance;

		bool enableValidationLayers;
		VkDebugUtilsMessengerEXT debugMessenger;
		
		VkSurfaceKHR surface;

		VkPhysicalDevice physicalDevice;
		VkDevice device;

		QueueFamilyIndices queueFamilyIndices;
		VkQueue graphicsQueue;
		VkQueue presentQueue;

		const std::vector<const char*> globalExtensions = {

		};
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};
	};
}