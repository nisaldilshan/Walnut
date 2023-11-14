#pragma once


namespace Walnut {

	int Main(int argc, char** argv)
	{
		while (g_ApplicationRunning) // This is an extern variable in Application.h
		{
			Walnut::Application* app = Walnut::CreateApplication(argc, argv); // This is a function declared in Application.h
			app->Run();
			delete app;
		}

		return 0;
	}

}

int main(int argc, char** argv)
{
	return Walnut::Main(argc, argv);
}
