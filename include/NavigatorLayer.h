#pragma once

#include "Battery/Battery.h"
#include "Navigator.h"

class NavigatorLayer : public Battery::Layer {
public:

	NavigatorLayer() {
		Navigator::CreateInstance();
	}

	~NavigatorLayer() {
		Navigator::DestroyInstance();
	}

	void OnAttach() override {
		Navigator::GetInstance()->OnAttach();
	}

	void OnDetach() override {
		Navigator::GetInstance()->OnDetach();
	}

	void OnUpdate() override {
		Navigator::GetInstance()->OnUpdate();
	}

	void OnRender() override {
		Navigator::GetInstance()->OnRender();
	}

	void OnEvent(sf::Event e, bool& handled) override {
		Navigator::GetInstance()->OnEvent(e, handled);
	}
};
