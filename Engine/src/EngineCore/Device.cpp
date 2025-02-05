#include "pch.h"
#include "Device.h"

namespace Luxel
{
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	Device::Device(const char* appName, bool enableValidation, GLFWwindow* window) : enableValidationLayers(enableValidation)
	{
		CreateInstance(appName);
		SetupDebugMessenger();
		CreateSurface(window);
		PickupPhysicaclDevice();
		CreateLogicalDevice();
		CreateCommandPool();
	}

	Device::~Device()
	{
		Info("Unload device.");
		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	VkDevice Device::GetDevice()
	{
		return device;
	}

	VkPhysicalDevice Device::GetPhysicalDevice()
	{
		return physicalDevice;
	}

	VkSurfaceKHR Device::GetSurface()
	{
		return surface;
	}

	QueueFamilyIndices Device::GetQueueFamilyIndices()
	{
		return queueFamilyIndices;
	}

	void Device::CreateInstance(const char* appName)
	{
		Info("Create Vulkan Instance.");
		// create appInfo
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// create createInfo
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// get glfw required extensions
		ui32 glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// merge all enable extensions
		std::vector<const char*> mergeExtentions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		for (const auto& extension : globalExtensions) {
			if (extension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME)continue;
			mergeExtentions.emplace_back(extension);
		}

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {

			if (!CheckValidationLayerSupport()) {
				Error("Validation layers not supported.");
				throw std::runtime_error("Validation layers not supported.");
			}

			mergeExtentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			PopulateDebugUtilsMessengerCreateInfo(debugCreateInfo);

			// do not define validationLayers here.
			createInfo.enabledLayerCount = static_cast<ui32>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
		}

		createInfo.enabledExtensionCount = static_cast<ui32>(mergeExtentions.size());
		createInfo.ppEnabledExtensionNames = mergeExtentions.data();

		Info(mergeExtentions.size(), "global extensions loaded.");
		for (const auto extension : mergeExtentions) {
			Info("Load global extension:", extension);
		}

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

		if (result != VK_SUCCESS) {
			Error("Failed to create Vulkan Instance.");
			throw std::runtime_error("Failed to create Vulkan instance.");
		}
	}

	void Device::SetupDebugMessenger()
	{
		if (!enableValidationLayers)return;
		Info("Setup Validation layers.");
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugUtilsMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger)
			!= VK_SUCCESS) {
			Error("Failed to setup debug messenger.");
			throw std::runtime_error("Failed to set up debug messenger.");
		}
	}

	void Device::CreateSurface(GLFWwindow* window)
	{
		Info("Create GLFW Window Surface.");
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			Error("Failed to create surface.");
			throw std::runtime_error("Failed to create surface.");
		 }
	}

	void Device::PickupPhysicaclDevice()
	{
		Info("Pick up physical device.");
		ui32 deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			Error("no avaliable device.");
			throw std::runtime_error("failed to pick up a avaliable device.");
		}

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
		
		Debug(deviceCount, "physical devices supported.");
		for (const auto& device : physicalDevices) {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(device, &properties);

			Debug("Avaliable physical device:", properties.deviceName);
		}

		for (const auto& device : physicalDevices) {
			if (CheckPhysicalDevice(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == nullptr) {
			Error("no suitable device.");
			throw std::runtime_error("no suitable device to use.");
		}
		else {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physicalDevice, &properties);
			Info("Use device:", properties.deviceName);
		}
	}

	void Device::CreateLogicalDevice()
	{
		Info("Create Logical Device.");
		queueFamilyIndices = FindQueueFamilies(physicalDevice, true);

		// create queues
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
		std::multiset<ui32> queueFamilyValue = {
			queueFamilyIndices.graphicsFamily.value(),     // add graphics queue
			queueFamilyIndices.presentFamily.value()       // add present queue
		};

		// create queue
		float queuePriority = 1.0f;
		for (ui32 index : queueFamilyValue) {
			VkDeviceQueueCreateInfo createInfo{};
			GetDeviceQueueCreateInfo(createInfo, index, 1, &queuePriority);
			queueCreateInfo.push_back(createInfo);
		}

		// create used device features
		VkPhysicalDeviceFeatures deviceFeatures{};

		// create vk device
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<ui32>(queueCreateInfo.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		
		// set vk device extensions/layers
		const std::vector<const char*> deviceExtensions =
		{ 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME 
		};
		deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		if (enableValidationLayers) {
			deviceCreateInfo.enabledLayerCount = 1;
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}

		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
			Error("Failed to create logical device.");
			throw std::runtime_error("Failed to create logical device.");
		}

		Info(deviceExtensions.size(), "device extensions loaded.");
		for (const auto& extension : deviceExtensions) {
			Info("Load device extension:", extension);
		}

		// create handle of queues
		vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
	}

	void Device::CreateCommandPool()
	{
		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		// not implement right now.
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Device::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		{
			switch (messageSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				Debug(pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				Info(pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				Warning(pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				Error(pCallbackData->pMessage);
				break;
			default:
				Fatal(pCallbackData->pMessage);
				break;
			}

			return VK_FALSE;
		}
	}

	VkResult Device::CreateDebugUtilsMessengerEXT(
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void Device::DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator
	)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	void Device::PopulateDebugUtilsMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo.sType =
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device, bool outputLog)
	{
		QueueFamilyIndices indices;

		ui32 queueFamiliesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilies.data());

		int i = 0;
		if (outputLog) {
			Debug(queueFamiliesCount, "queue families supported.");
		}
		for (const auto& queueFamily : queueFamilies) {
			VkBool32 presentSupported = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupported);

			std::string queueType = GetQueueType(queueFamily.queueFlags) + (presentSupported ? " | present" : "");
			if (outputLog) {
				Debug("Avaliable queue family:", queueType, "count:", queueFamily.queueCount);
			}

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			if (presentSupported) {
				indices.presentFamily = i;
			}
			i++;
		}

		return indices;
	}

	void Device::GetDeviceQueueCreateInfo(VkDeviceQueueCreateInfo& queueCreateInfo, const ui32 queueFamilyIndex, const ui32 queueCount, const float* priority)
	{
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = queueCount;
		queueCreateInfo.pQueuePriorities = priority;
	}

	std::string Device::GetQueueType(const uint32_t queueFlag) const{
		std::string result = "";

		if (queueFlag & VK_QUEUE_GRAPHICS_BIT) {
			result += "graphics";
		}

		if (queueFlag & VK_QUEUE_COMPUTE_BIT) {
			if (!result.empty()) result += " | ";
			result += "compute ";
		}

		if (queueFlag & VK_QUEUE_TRANSFER_BIT) {
			if (!result.empty()) result += " | ";
			result += "transfer";
		}

		if (queueFlag & VK_QUEUE_SPARSE_BINDING_BIT) {
			if (!result.empty()) result += " | ";
			result += "sparse binding";
		}

		if (result.empty()) {
			return "Unknown Queue Type";
		}

		return result;
	}


	bool Device::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	bool Device::CheckPhysicalDevice(const VkPhysicalDevice& device)
	{
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;

		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);

		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device, false);

		// check device extension supported.
		ui32 extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> supportedDeviceExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, supportedDeviceExtensions.data());
		std::set<std::string> requiredDeviceExtensinos(deviceExtensions.begin(), deviceExtensions.end());
		for (const auto& extension : supportedDeviceExtensions) {
			requiredDeviceExtensinos.erase(extension.extensionName);
		}

		return
			features.geometryShader &&
			properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			queueFamilyIndices.isComplete() &&
			requiredDeviceExtensinos.empty();
	}
}