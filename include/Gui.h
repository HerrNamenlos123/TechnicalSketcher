#pragma once

#include "Battery/Battery.h"
#include "config.h"
#include "Navigator.h"

namespace GUI {

	class RibbonWindow : public Battery::StaticImGuiWindow<> {
	public:

		RibbonWindow() : StaticImGuiWindow("RibbonWindow", { 0, 0 }, { 0, 0 }) {
		}

		void OnAttach() {
		}

		void OnDetach() {
		}

		void OnUpdate() override {
			windowSize.x = applicationPointer->window.GetWidth();
			windowSize.y = GUI_RIBBON_HEIGHT;
		}

		void OnRender() override {

		}
	};

	class LayerWindow : public Battery::StaticImGuiWindow<> {

		bool wasMouseOnWindow = false;	// For the mouse enter event

	public:

		// Storage
		LayerID selectedLayer = -1;
		LayerID moveLayerBackID = -1;
		LayerID moveLayerFrontID = -1;

		// Flags
		bool addLayerFlag = false;
		bool layerSelectedFlag = false;
		bool moveLayerFrontFlag = false;
		bool moveLayerBackFlag = false;
		bool mouseEnteredWindowFlag = false;

		ImFont* font = nullptr;

		LayerWindow() : StaticImGuiWindow("LayerWindow", { 0, GUI_RIBBON_HEIGHT }, 
			{ GUI_LEFT_BAR_WIDTH, GUI_LAYER_WINDOW_HEIGHT }, 
			DEFAULT_STATIC_IMGUI_WINDOW_FLAGS | ImGuiWindowFlags_NoBringToFrontOnFocus) {
		}

		void OnAttach() {
			ImGuiIO& io = ImGui::GetIO();
			font = io.Fonts->AddFontFromFileTTF(GUI_FONT, 20);
		}

		void OnDetach() {
			// Font does not need to be deleted
		}

		void OnUpdate() override {

		}

		void OnRender() override {

			ImGui::PushFont(font);

			ImGui::Text("Layers"); ImGui::SameLine();
			ImGui::SetCursorPosX(GUI_LEFT_BAR_WIDTH * 0.8);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

			// Set flag, which will be read and acted on in the main loop event handler
			addLayerFlag = ImGui::Button("+##AddLayer", ImVec2(25, 17));

			ImGui::PopStyleVar();
			ImGui::Separator();

			float fontHeight = 22;					// Font height
			float itemHeight = fontHeight + 4;		// Font height + 4 pixels
			ImGuiIO& io = ImGui::GetIO();

			bool anyActive = false;

			::App* app = GetApp();	// Get a pointer to the global application class
			SketchFile* file = &app->navigator->file;	// Get reference to the current file
			
			for (Layer* layer : file->GetLayers()) {

				std::string name = layer->name + "##" + std::to_string(layer->layerID);

				// Draw GUI element and if clicked save flag, which will be read from the main loop event handler
				if (ImGui::Selectable(name.c_str(), file->GetActiveLayerID() == layer->layerID)) {
					selectedLayer = layer->layerID;
					layerSelectedFlag = true;
				}

				// Remember if any selectable is hovered
				if (ImGui::IsItemHovered())
					anyActive = true;

				// Drag and drop
				if (ImGui::IsItemActive() && !ImGui::IsItemHovered() && isMouseOnWindow) {
					if (ImGui::GetMouseDragDelta(0).y < 0.f)
					{
						moveLayerFrontID = layer->layerID;
						moveLayerFrontFlag = true;
						ImGui::ResetMouseDragDelta();
					}
					else {
						moveLayerBackID = layer->layerID;
						moveLayerBackFlag = true;
						ImGui::ResetMouseDragDelta();
					}
				}

				// Draw the preview of the layer
				if (ImGui::IsItemHovered()) {
					LOG_WARN("DRAW PREVIEW NOW");
					//if (layer->bitmap != nullptr) {
					//	ImGui::BeginTooltip();
					//	ImGui::Image(layer->bitmap, ImVec2(GUI_PREVIEWWINDOW_SIZE, GUI_PREVIEWWINDOW_SIZE));
					//	ImGui::EndTooltip();
					//}
					//else {
					//	std::cout << "WARNING: Bitmap is nullptr!!!" << std::endl;
					//}
				}
			}

			if (anyActive) {
				moveLayerFrontFlag = false;
				moveLayerBackFlag = false;
			}

			if (isMouseOnWindow && !wasMouseOnWindow) {
				mouseEnteredWindowFlag = true;
			}
			wasMouseOnWindow = isMouseOnWindow;

			ImGui::PopFont();
		}
	};

	class ToolboxWindow : public Battery::StaticImGuiWindow<> {
	public:

		ImFont* font = nullptr;

		ToolboxWindow() : StaticImGuiWindow("ToolboxWindow", { 0, GUI_RIBBON_HEIGHT + GUI_LAYER_WINDOW_HEIGHT }, 
			{ GUI_LEFT_BAR_WIDTH, GUI_TOOLBOX_WINDOW_HEIGHT }, 
			DEFAULT_STATIC_IMGUI_WINDOW_FLAGS | ImGuiWindowFlags_NoBringToFrontOnFocus) {
		}

		void OnAttach() {
			ImGuiIO& io = ImGui::GetIO();
			font = io.Fonts->AddFontFromFileTTF(GUI_FONT, 16);
		}

		void OnDetach() {
			// Font does not need to be deleted
		}

		void OnUpdate() override {
		}

		void OnRender() override {

			ImGui::PushFont(font);
			Navigator* nav = GetApp()->navigator;

			if (ImGui::Selectable("Selection mode", nav->selectedTool == CursorTool::SELECT)) {
				nav->UseTool(CursorTool::SELECT);
			}
			if (ImGui::Selectable("Line mode", nav->selectedTool == CursorTool::LINE)) {
				nav->UseTool(CursorTool::LINE);
			}
			if (ImGui::Selectable("Line strip mode", nav->selectedTool == CursorTool::LINE_STRIP)) {
				nav->UseTool(CursorTool::LINE_STRIP);
			}

			ImGui::PopFont();
		}
	};

	class MouseInfoWindow : public Battery::StaticImGuiWindow<> {
	public:

		ImFont* font = nullptr;

		MouseInfoWindow() : StaticImGuiWindow("MouseInfoWindow", { 0, 0 }, 
			{ GUI_LEFT_BAR_WIDTH, GUI_MOUSEINFO_WINDOW_HEIGHT }) {
		}

		void OnAttach() {
			ImGuiIO& io = ImGui::GetIO();
			font = io.Fonts->AddFontFromFileTTF(GUI_FONT, 8);
		}

		void OnDetach() {
			// Font does not need to be deleted
		}

		void OnUpdate() override {
			windowPosition.y = applicationPointer->window.GetHeight() - GUI_MOUSEINFO_WINDOW_HEIGHT;
		}

		void OnRender() override {

			ImGui::PushFont(font);
			Navigator* nav = GetApp()->navigator;

			std::stringstream text;
			text << std::fixed << std::setprecision(2) << "Mouse: ";
			if (nav->mousePosition.x >= 0)
				text << " ";
			text << nav->mousePosition.x << "|";
			if (nav->mousePosition.y >= 0)
				text << " ";
			text << nav->mousePosition.y << " Snap: " << std::setprecision(0);
			if (nav->mouseSnapped.x >= 0)
				text << " ";
			text << nav->mouseSnapped.x << "|";
			if (nav->mouseSnapped.y >= 0)
				text << " ";
			text << nav->mouseSnapped.y;
			ImGui::Text(text.str().c_str());

			ImGui::PopFont();
		}
	};

	class GuiLayer : public Battery::ImGuiLayer {
	public:

		RibbonWindow ribbon;
		LayerWindow layers;
		ToolboxWindow toolbox;
		MouseInfoWindow mouseInfo;

		GuiLayer() : Battery::ImGuiLayer("GuiLayer") {
		}

		~GuiLayer() {
		}

		void OnImGuiAttach() override {
			ImGui::GetIO().IniFilename = NULL;	// Prevent ImGui from saving a .ini file

			ribbon.OnAttach();
			layers.OnAttach();
			toolbox.OnAttach();
			mouseInfo.OnAttach();
		}

		void OnImGuiDetach() override {

			ribbon.OnDetach();
			layers.OnDetach();
			toolbox.OnDetach();
			mouseInfo.OnDetach();
		}

		void OnImGuiUpdate() override {
			ribbon.Update();
			layers.Update();
			toolbox.Update();
			mouseInfo.Update();
		}

		void OnImGuiRender() override {
			ribbon.Render();
			layers.Render();
			toolbox.Render();
			mouseInfo.Render();
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

}