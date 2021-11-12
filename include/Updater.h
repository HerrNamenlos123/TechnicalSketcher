#pragma once

#include "pch.h"
#include "config.h"
#include "Navigator.h"

extern void RunUpdater();

class UpdaterLayer : public Battery::Layer {
public:

	std::thread updaterThread;

	UpdaterLayer() : Battery::Layer("UpdaterLayer") {}

	void OnAttach() override {

#ifdef BATTERY_DEPLOY
		if (Navigator::GetInstance()->keepUpToDate) {
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