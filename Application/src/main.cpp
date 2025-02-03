#include <Engine.h>

class MainWindow : public Luxel::Application
{
public:
	MainWindow()
	{

	}

	~MainWindow()
	{

	}
};

Luxel::Application* Luxel::CreateApplication()
{
	return new MainWindow();
}