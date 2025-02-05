#pragma once

#include "pch.h"

#include "Core.h"

#include "log.h"
#include "RenderPipeline.h"
#include "Device.h"

namespace Luxel
{
	class LUXEL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Init(int width, int height, const char* name);
		void Run();

	private:
		GLFWwindow* window;
		RenderPipeline* renderPipeline;
		Device* device;
	};

	Application* CreateApplication();
}