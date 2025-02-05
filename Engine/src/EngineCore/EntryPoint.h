#pragma once

#ifdef LUXEL_PLATFORM_WINDOWS

extern Luxel::Application* Luxel::CreateApplication();

using namespace Luxel;

int main(int argc, char** argv)
{
	Info("Start Luxel Engine.");
	auto window = CreateApplication();
	window->Init(1024, 1024, "Luxel Engine");
	window->Run();
	delete window;	
}

#endif