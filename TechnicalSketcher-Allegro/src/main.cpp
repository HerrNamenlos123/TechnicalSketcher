
#include "pch.h"
#include "Application.h"
#include "config.h"

int main() {

	{
		Application app;
		app.run(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_FLAGS);

		std::cout << "Shutting down..." << std::endl;
	}

	std::cout << "Application destroyed" << std::endl;

	return 0;
}