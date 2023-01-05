#pragma once

#include "pch.h"
#include "config.h"
#include "Navigator.h"

extern void RunUpdater();

class UpdaterLayer : public Battery::Layer {
public:

	std::thread updaterThread;
	bool noUpdate;

	UpdaterLayer(bool noUpdate) : noUpdate(noUpdate) {}

	void OnAttach() override {

#ifdef BATTERY_DEPLOY
		if (Navigator::GetInstance()->keepUpToDate && !noUpdate) {
			updaterThread = std::thread(RunUpdater);		// Updater will run in the background
		}
#endif

	}

	void OnDetach() override {
		if (updaterThread.joinable()) {
			updaterThread.join();
		}
	}
};