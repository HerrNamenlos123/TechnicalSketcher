#pragma once

#include "pch.h"
#include "Navigator.h"

class App : public Battery::Application {
public:

	double lastScreenRefresh = 0;
	double passiveScreenTime = 5.0;		// Refresh the screen every 2 seconds
	ALLEGRO_FONT* font = nullptr;

	//std::string loadingMessage = "";

	App();

	bool OnStartup() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnShutdown() override;
	void OnEvent(Battery::Event* e) override;
};
