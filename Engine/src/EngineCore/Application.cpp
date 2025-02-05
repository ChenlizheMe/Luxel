#include "pch.h"

#include "Application.h"

namespace Luxel
{
	Application::Application()
	{
		
	}

	Application::~Application()
	{
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
		std::vector<const char*> enableExtensions;
		//device = new Device(name, true, enableExtensions, window);
		device = new Device{ name, true, enableExtensions, window };

		// create render pipeline
		Info("Create render pipeline.");
		renderPipeline = new RenderPipeline{
			device,
			"./shaders/simple_shader.vert.spv",
			"./shaders/simple_shader.frag.spv",
			RenderPipeline::DefaultPipelineConfigInfo(width, height)
		};
	}

	void Application::Run()
	{
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}
}