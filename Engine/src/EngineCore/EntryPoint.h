#pragma once

#ifdef LUXEL_PLATFORM_WINDOWS

extern Luxel::Application* Luxel::CreateApplication();

int main(int argc, char** argv)
{
	auto window = Luxel::CreateApplication();
	window->Run();
	delete window;	
}

#endif