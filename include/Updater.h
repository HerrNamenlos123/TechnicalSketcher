#pragma once

#include "Battery/Battery.h"
#include "config.h"

static void RunUpdater() {
	using namespace Battery;

#ifndef BATTERY_DEBUG
	try {
		std::vector<std::string> components = FileUtils::GetComponentsFromPath(FileUtils::GetExecutableDirectory());
		if (components.size() > 0) {
			components.pop_back();	// Remove last component
			std::string path = StringUtils::JoinStrings(components, "/");
			system((std::string("cd ") + path + " && " UPDATER_EXE_NAME).c_str());
		}
	}
	catch (const Battery::Exception& e) {
		LOG_CORE_CRITICAL(std::string("The Updater thread threw Battery::Exception: ") + e.what());
		ShowErrorMessageBox(std::string("The Updater thread threw Battery::Exception: ") + e.what());
		return;
	}
	catch (...) {
		LOG_CORE_CRITICAL("The Updater thread threw unknown Exception: No further information");
		ShowErrorMessageBox("The Updater thread threw unknown Exception: No further information");
		return;
	}
#endif
}

class UpdaterLayer : public Battery::Layer {
public:

	std::thread* updaterThread = nullptr;

	UpdaterLayer() : Battery::Layer("UpdaterLayer") {}

	~UpdaterLayer() {
		if (updaterThread != nullptr) {
			updaterThread->join();
			delete updaterThread;
			updaterThread = nullptr;
		}
	}

	void OnAttach() override {
		updaterThread = new std::thread(RunUpdater);	// Updater will run in the background
	}

	void OnDetach() override {
		updaterThread->join();
		delete updaterThread;
		updaterThread = nullptr;
	}
};