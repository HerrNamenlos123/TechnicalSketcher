#pragma once

#include "pch.h"
#include "config.h"

static void RunUpdater() {
	using namespace Battery;

#ifndef BATTERY_DEBUG
	try {
		std::vector<std::string> components = FileUtils::GetComponentsFromPath(FileUtils::GetExecutableDirectory());
		if (components.size() > 0) {
			components.pop_back();	// Remove last component
			std::string updater = StringUtils::JoinStrings(components, "/") + "/" + UPDATER_EXE_NAME;
			std::string log = Navigator::GetInstance()->GetSettingsDirectory() + "update.log";

			if (Battery::FileUtils::FileExists(updater)) {
				WinExec((updater + " > " + log).c_str(), SW_SHOW);
			}
			else {
				// Now try the fixed path
				auto paths = Battery::FileUtils::GetComponentsFromPath(Navigator::GetInstance()->GetSettingsDirectory());
				if (paths.size() >= 3) {
					paths.pop_back();
					paths.pop_back();

					updater = Battery::StringUtils::JoinStrings(paths, "/") + "/Local/" + APPLICATION_NAME + "/updater.exe";
					if (Battery::FileUtils::FileExists(updater)) {
						WinExec((updater + " > " + log).c_str(), SW_SHOW);
					}
					else {
						Battery::FileUtils::WriteFile(log, "Updater executable cannot be found");
					}
				}
				else {
					Battery::FileUtils::WriteFile(log, "Updater executable cannot be found");
				}
			}
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

	std::unique_ptr<std::thread> updaterThread;

	UpdaterLayer() : Battery::Layer("UpdaterLayer") {}

	~UpdaterLayer() {
		if (updaterThread) {
			updaterThread->join();
		}
	}

	void OnAttach() override {
		if (Navigator::GetInstance()->keepUpToDate) {
			updaterThread = std::make_unique<std::thread>(RunUpdater);	// Updater will run in the background
		}
	}

	void OnDetach() override {
		if (updaterThread) {
			updaterThread->join();
			updaterThread.release();
		}
	}
};