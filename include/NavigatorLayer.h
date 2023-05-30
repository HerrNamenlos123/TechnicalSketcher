#pragma once

#include "Navigator.h"

class NavigatorLayer {
public:

	NavigatorLayer() {
		Navigator::CreateInstance();
	}

	~NavigatorLayer() {
		Navigator::DestroyInstance();
	}

	void OnAttach() {
		Navigator::GetInstance()->OnAttach();
	}

	void OnDetach() {
		Navigator::GetInstance()->OnDetach();
	}

	void OnUpdate() {
		Navigator::GetInstance()->OnUpdate();
	}

	void OnRender() {
		Navigator::GetInstance()->OnRender();
	}

	void OnEvent(sf::Event e, bool& handled) {
		Navigator::GetInstance()->OnEvent(e, handled);
	}
};
