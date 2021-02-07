#pragma once

#include "Battery/Battery.h"
#include "../TechnicalSketcher/resource.h"
#include "config.h"
#include "PerlinNoise.h"

static void updateApplication(bool dev) {
	using namespace Battery::FileUtils;
	using namespace Battery::StringUtils;

	if (!dev) {
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
}

static std::string getVersion(bool dev) {
	using namespace Battery::FileUtils;
	using namespace Battery::StringUtils;

	std::vector<std::string> comp = GetComponentsFromPath(GetExecutableDirectory());
	if (comp.size() > 0) {

		comp.pop_back();	// Remove last component
		if (dev)
			comp.pop_back();

		std::string versionFile = JoinStrings(comp, "/") + "/version";
		File file = LoadFile(versionFile);
		if (!file.fail()) {
			return file.str();
		}
	}

	return "";
}

class Application : public Battery::Engine {
public:

	std::thread* updaterThread = nullptr;
	std::string applicationVersion;

	std::string filename = "Unnamed";
	bool fileChanged = false;
	bool devLaunch = false;

	uint32_t oldTime = 0;

	siv::PerlinNoise perlin;



	Application();
	
	bool Setup(std::vector<std::string> args) override;

	bool Update() override;

	// This function is used to shut the engine down
	void Shutdown() override;

	// This function is called after the engine was shut down 
	// and the main window is closed,
	// last function before main() returns
	void PostApplicationFunction() override;

	void KeyPressed(int keycode, char unicode, int modifiers, bool repeat) override;



	void Line(glm::vec2 p1, glm::vec2 p2, float thickness, float outline);

	void updateWindowTitle();

	std::string readRecentFile();

	bool writeRecentFile(std::string content);

};
