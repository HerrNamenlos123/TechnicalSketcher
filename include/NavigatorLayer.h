#pragma once

#include "Battery/Battery.h"
#include "Navigator.h"

class NavigatorLayer : public Battery::Layer {
public:

	NavigatorLayer() : Battery::Layer("NavigatorLayer") {
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

	void OnEvent(Battery::Event* e) override {
		Navigator::GetInstance()->OnEvent(e);
	}
};
