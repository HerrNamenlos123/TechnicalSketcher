#pragma once

#include "pch.h"
#include "fonts.h"

/// <summary>
/// Contains core logic for an ImGui window and remembers flags, but never saves a .ini file
/// to reset everything when program is reloaded.
/// This class is supposed to be derived from.
/// Template can be empty, any template parameters will appear in the update functions and will be simply transferred to the callback function.
/// </summary>
template<typename... Targs>
class StaticImGuiWindow {

	bool firstUpdate;

protected:

	bool imgui_no_titlebar = false;
	bool imgui_no_scrollbar = false;
	bool imgui_no_move = false;
	bool imgui_no_resize = false;
	bool imgui_no_menu = true;
	bool imgui_no_collapse = false;
	bool imgui_no_nav = false;
	bool imgui_no_background = false;
	bool imgui_no_bring_to_front = false;

public:

	bool isMouseOnWindow = false;
	glm::vec2 windowPosition;
	glm::vec2 windowSize;
	std::string name;

	/// <summary>
	/// '_name' is an ImGui window name and must be unique. As long as ImGuiWindowFlags_NoMove flag is NOT set, 
	/// position is only set once on startup, same for ImGuiWindowFlags_NoResize and the size parameter.
	/// </summary>
	StaticImGuiWindow(const std::string& _name, glm::vec2 position, glm::vec2 size, ImGuiWindowFlags flags = 0) {
		name = _name;

		windowPosition = position;
		windowSize = size;
		firstUpdate = true;

		if ((flags & ImGuiWindowFlags_NoTitleBar) > 0) imgui_no_titlebar = true;
		if ((flags & ImGuiWindowFlags_NoScrollbar) > 0) imgui_no_scrollbar = true;
		if ((flags & ImGuiWindowFlags_NoMove) > 0) imgui_no_move = true;
		if ((flags & ImGuiWindowFlags_NoResize) > 0) imgui_no_resize = true;
		if ((flags & ImGuiWindowFlags_MenuBar) > 0) imgui_no_menu = false;
		if ((flags & ImGuiWindowFlags_NoCollapse) > 0) imgui_no_collapse = true;
		if ((flags & ImGuiWindowFlags_NoNav) > 0) imgui_no_nav = true;
		if ((flags & ImGuiWindowFlags_NoBackground) > 0) imgui_no_background = true;
		if ((flags & ImGuiWindowFlags_NoBringToFrontOnFocus) > 0) imgui_no_bring_to_front = true;
	}


protected:

	/// <summary>
	/// This function is called in ImGuiWindow::updateStaticImGuiWindow() and is supposed to be
	/// overridden by a derived class. Should contain the code for any GUI elements in the window.
	/// </summary>
	virtual void updateWindow(Targs... args) {
	}
	
	/// <summary>
	/// Updates the ImGui window and calls ImGuiWindow::updateWindow(), which is supposed to be overridden
	/// by a derived class
	/// </summary>
	void updateStaticImGuiWindow(Targs... args) {

		ImGuiWindowFlags window_flags = 0;
		if (imgui_no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
		if (imgui_no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
		if (imgui_no_move)			  window_flags |= ImGuiWindowFlags_NoMove;
		if (imgui_no_resize)		  window_flags |= ImGuiWindowFlags_NoResize;
		if (!imgui_no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
		if (imgui_no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
		if (imgui_no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
		if (imgui_no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
		if (imgui_no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		window_flags |= ImGuiWindowFlags_NoSavedSettings;

		if (imgui_no_move || firstUpdate)
			ImGui::SetNextWindowPos(ImVec2(windowPosition.x, windowPosition.y));

		if (imgui_no_resize || firstUpdate)
			ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y));

		ImGui::GetStyle().WindowRounding = 0.0f;

		bool* p_open = nullptr;	// Hide close button
		ImGui::Begin(name.c_str(), p_open, window_flags);

		// Check if mouse in on window
		isMouseOnWindow = ImGui::GetMousePos().x >= ImGui::GetWindowPos().x &&
						  ImGui::GetMousePos().x <= ImGui::GetWindowPos().x + ImGui::GetWindowSize().x &&
						  ImGui::GetMousePos().y >= ImGui::GetWindowPos().y &&
						  ImGui::GetMousePos().y <= ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;

		// Virtual function, overridden by a derived class
		updateWindow(args...);

		ImGui::End();

		firstUpdate = false;
	}
};
