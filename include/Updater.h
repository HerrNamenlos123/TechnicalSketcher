#pragma once

#include "pch.hpp"
#include "TskConfig.hpp"
#include "TskHost.hpp"

extern void RunUpdater();

class UpdaterLayer {
public:

	b::thread updaterThread;
	bool noUpdate;

	UpdaterLayer(bool noUpdate) : noUpdate(noUpdate) {}

	void OnAttach() {

#ifdef BATTERY_DEPLOY
		if (Navigator::GetInstance()->keepUpToDate && !noUpdate) {
			updaterThread = std::thread(RunUpdater);		// Updater will run in the background
		}
#endif

	}

	void OnDetach() {
		if (updaterThread.joinable()) {
			updaterThread.join();
		}
	}
};