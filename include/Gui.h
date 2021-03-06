#pragma once

#include "Battery/Battery.h"

class Window : public Battery::StaticImGuiWindow<> {
public:
	Battery::Application* app = nullptr;

	Window() : StaticImGuiWindow("ImGuiWindow", { 0, 0 }, { 200, 0 }) {
	}

	void OnUpdate() override {
		app = Battery::Application::GetApplicationPointer();
		windowSize.y = app->window.GetHeight();
	}

	void OnRender() override {

	}
};

class GuiLayer : public Battery::ImGuiLayer {
public:

	Window imguiWindow;

	GuiLayer() : Battery::ImGuiLayer("GuiLayer") {
	}

	~GuiLayer() {
	}

	void OnImGuiAttach() override {
		ImGui::GetIO().IniFilename = NULL;	// Prevent ImGui from saving a .ini file
	}

	void OnImGuiDetach() override {

	}

	void OnImGuiUpdate() override {
		imguiWindow.Update();
	}

	void OnImGuiRender() override {
		imguiWindow.Render();
	}

	void OnImGuiEvent(Battery::Event* e) override {

		// Prevent events from propagating through to the application
		switch (e->GetType()) {

			// Mouse related events
		case Battery::EventType::MouseButtonPressed:
		case Battery::EventType::MouseButtonReleased:
		case Battery::EventType::MouseMoved:
		case Battery::EventType::MouseScrolled:
			if (io.WantCaptureMouse) {
				e->SetHandled();
				return;
			}
			break;

			// Keyboard related events
		case Battery::EventType::KeyPressed:
		case Battery::EventType::KeyReleased:
		case Battery::EventType::TextInput:
			if (io.WantCaptureKeyboard || io.WantTextInput) {
				e->SetHandled();
				return;
			}
			break;
		}
	}
};