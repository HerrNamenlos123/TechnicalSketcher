#pragma once

#include "Battery.h"

class Application : public Battery::Engine {
public:
	
	bool Setup() override {
		return true;
	}

	bool Update() override {
		using namespace Battery::Graphics;
		
		DrawBackground({ 255, 255, 255 });

		ImGui::Begin("Test");
		ImGui::End();

		return true;
	}

	void Shutdown() override {

	}

};
