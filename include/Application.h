#pragma once

#include "Battery.h"
#include "../TechnicalSketcher/resource.h"

void updateApplication() {
	using namespace Battery::FileUtils;
	using namespace Battery::StringUtils;

	std::vector<std::string> comp = GetComponentsFromPath(GetExecutableDirectory());
	if (comp.size() > 0) {
		comp.pop_back();	// Remove last component
		std::string path = JoinStrings(comp, "/");
		system((std::string("cd ") + path + " && updater.exe").c_str());
	}
	else {
		std::cout << "Can't find path of the updater" << std::endl;
	}
}

class Application : public Battery::Engine {
public:

	std::thread* updaterThread;
	
	bool Setup() override {
		using namespace Battery::FileUtils;
		using namespace Battery::StringUtils;

		// Prevent ImGui from saving a .ini file
		ImGui::GetIO().IniFilename = NULL;

		// Start updater in background
		updaterThread = new std::thread(updateApplication);

		// Set window icon
		SetWindowExecutableIcon();

		return true;
	}

	bool Update() override {
		using namespace Battery::Graphics;
		
		DrawBackground({ 0, 0, 0 });

		ImGui::Begin("Test");
		ImGui::End();

		return true;
	}

	// This function is used to shut the engine down
	void Shutdown() override {

	}

	// This function is called after the engine is completely shut down already,
	// last function before main() returns
	void PostApplicationFunction() override {
		updaterThread->join();
	}

};
