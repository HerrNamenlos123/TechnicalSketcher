#pragma once

#include "pch.h"

class BatteryApp : public Battery::Application {
public:

	sf::Time lastScreenRefresh = sf::seconds(0);
	sf::Time passiveScreenTime = sf::seconds(5.0);

	BatteryApp() {}

	void OnStartup() override;
	void OnUpdate() override;
	void OnEvent(sf::Event e, bool& handled) override;
};
