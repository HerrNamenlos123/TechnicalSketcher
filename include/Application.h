#pragma once
/*

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
}*/

#include "pch.h"
#include "Navigator.h"

class App : public Battery::Application {
public:

	double lastScreenRefresh = 0;
	double passiveScreenTime = 5.0;		// Refresh the screen every 2 seconds

	std::string loadingMessage = "";

	App();

	bool OnStartup() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnShutdown() override;
	void OnEvent(Battery::Event* e) override;
};

App* GetClientApplication();
