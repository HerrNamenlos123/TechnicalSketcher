#pragma once

#include "Battery.h"

class Application : public Battery::Engine {
public:
	
	bool Setup() override {

		// Prevent ImGui from saving a .ini file
		ImGui::GetIO().IniFilename = NULL;

		return true;
	}

	bool Update() override {
		using namespace Battery::Graphics;
		
		DrawBackground({ 0, 0, 0 });

		ImGui::Begin("Test");
		ImGui::End();

		return true;
	}

	void Shutdown() override {

	}

};
