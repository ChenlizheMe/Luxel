#pragma once

#include "Core.h"

namespace Luxel
{
	class LUXEL_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
	};

	Application* CreateApplication();
}