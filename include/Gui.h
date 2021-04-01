#pragma once

#include "pch.h"
#include "config.h"
#include "Navigator.h"
#include "Tools/GenericTool.h"

namespace GUI {

	class RibbonWindow : public Battery::StaticImGuiWindow<> {
	public:

		RibbonWindow() : StaticImGuiWindow("RibbonWindow", { 0, 0 }, { 0, 0 }, 
			DEFAULT_STATIC_IMGUI_WINDOW_FLAGS | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus) {
		}

		void OnAttach() {
		}

		void OnDetach() {
		}

		void OnUpdate() override {
			windowSize.x = applicationPointer->window.GetWidth();
			windowSize.y = GUI_RIBBON_HEIGHT;
		}

		void MenuTab() {

			ImGui::MenuItem("TechnicalSketcher", NULL, false, false);
			if (ImGui::MenuItem("New")) {
				StartNewApplicationInstance();
			}
			if (ImGui::MenuItem("Open", "Ctrl+O")) {
			
			}
			if (ImGui::BeginMenu("Open Recent")) {
				ImGui::MenuItem("fish_hat.c");
				ImGui::MenuItem("fish_hat.inl");
				ImGui::MenuItem("fish_hat.h");
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S")) {
			
			}
			if (ImGui::MenuItem("Save As..", "Ctrl+Shift+S")) {
			
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Options")) {
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Quit", "Alt+F4")) {}
		}

		void EditTab() {
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
		}

		void MenuBar() {

			if (ImGui::BeginMainMenuBar()) {

				// File tab
				if (ImGui::BeginMenu("File")) {
					MenuTab();
					ImGui::EndMenu();
				}

				// Edit tab
				if (ImGui::BeginMenu("Edit")) {
					EditTab();
					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}
		}

		void OnRender() override {

			// Show the main menu bar
			MenuBar();

		}
	};

	class LayerWindow : public Battery::StaticImGuiWindow<> {

		bool wasMouseOnWindow = false;	// For the mouse enter event
		bool renameLayer = false;
		bool firstContext = true;

	public:

		// Storage
		LayerID selectedLayer = -1;
		LayerID moveLayerBackID = -1;
		LayerID moveLayerFrontID = -1;
		LayerID deleteLayer = -1;

		// Flags
		bool addLayerFlag = false;
		bool layerSelectedFlag = false;
		bool moveLayerFrontFlag = false;
		bool moveLayerBackFlag = false;
		bool deleteLayerFlag = false;
		bool mouseEnteredWindowFlag = false;

		ImFont* font20 = nullptr;
		ImFont* font17 = nullptr;
		//ImFont* font14 = nullptr;

		LayerWindow() : StaticImGuiWindow("LayerWindow", { 0, GUI_RIBBON_HEIGHT }, 
			{ GUI_LEFT_BAR_WIDTH, GUI_LAYER_WINDOW_HEIGHT }, 
			DEFAULT_STATIC_IMGUI_WINDOW_FLAGS | ImGuiWindowFlags_NoBringToFrontOnFocus) {
		}

		void OnAttach() {
			ImGuiIO& io = ImGui::GetIO();
			font20 = io.Fonts->AddFontFromFileTTF(GUI_FONT, 20);
			font17 = io.Fonts->AddFontFromFileTTF(GUI_FONT, 14);
			//font14 = io.Fonts->AddFontFromFileTTF(GUI_FONT, 14);
		}

		void OnDetach() {
			// Font does not need to be deleted
		}

		void OnUpdate() override {

		}

		void OnRender() override {

			if (!wasMouseOnWindow && isMouseOnWindow) {
				Navigator::GetInstance()->file.GeneratePreviews();
			}

			ImGui::PushFont(font20);

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

			SketchFile* file = &Navigator::GetInstance()->file;		// Get a reference to the current file

			for (auto& layer : file->GetLayers()) {
			
				char name[1024];
				snprintf(name, 1024, "%s##%zu", layer.name.c_str(), (size_t)layer.GetID());

				// Draw GUI element and if clicked save flag, which will be read from the main loop event handler
				if (ImGui::Selectable(name, file->GetActiveLayer().GetID() == layer.GetID())) {
					selectedLayer = layer.GetID();
					layerSelectedFlag = true;
				}

				// Context menu
				ContextMenu(layer);
			
				// Remember if any selectable is hovered
				if (ImGui::IsItemHovered())
					anyActive = true;
			
				// Drag and drop
				if (ImGui::IsItemActive() && !ImGui::IsItemHovered() && isMouseOnWindow) {
					if (ImGui::GetMouseDragDelta(0).y < 0.f)
					{
						moveLayerFrontID = layer.GetID();
						moveLayerFrontFlag = true;
						ImGui::ResetMouseDragDelta();
					}
					else {
						moveLayerBackID = layer.GetID();
						moveLayerBackFlag = true;
						ImGui::ResetMouseDragDelta();
					}
				}
			
				// Draw the preview of the layer
				if (ImGui::IsItemHovered()) {
					if (layer.previewImage.IsValid()) {
						ImGui::BeginTooltip();
						ImGui::Image(layer.previewImage.GetAllegroBitmap(), 
							ImVec2(GUI_PREVIEWWINDOW_SIZE, GUI_PREVIEWWINDOW_SIZE));
						ImGui::EndTooltip();
					}
					else {
						LOG_WARN("Bitmap is nullptr");
					}
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

		void ContextMenu(Layer& layer) {

			if (!renameLayer) {
				if (ImGui::BeginPopupContextItem()) {
					char rename[128];
					char remove[128];
					snprintf(rename, 128, "Rename##%zu", (size_t)layer.GetID());
					snprintf(remove, 128, "Delete##%zu", (size_t)layer.GetID());

					ImGui::PushFont(font17);
					ImGui::PushItemWidth(500); // TODO: Doesn't work

					// Rename layer
					if (ImGui::Button(rename)) {
						renameLayer = true;
					}

					// Delete layer
					if (ImGui::Button(remove)) {
						ImGui::OpenPopup("Delete Layer?");
					}

					// Modal window for deleting a layer
					ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
					ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
					if (ImGui::BeginPopupModal("Delete Layer?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
						char text[256];
						snprintf(text, 256, "Are you sure you want to \ndelete the Layer '%s'? "
							"\nThis can not be undone!\n\n", layer.name.c_str());
						ImGui::Text(text);
						ImGui::Separator();

						bool close = false;
						if (ImGui::Button("OK", ImVec2(120, 0))) {	// Delete the layer now
							ImGui::CloseCurrentPopup();
							deleteLayer = layer.GetID();
							deleteLayerFlag = true;
						}
						ImGui::SetItemDefaultFocus();
						ImGui::SameLine();
						if (ImGui::Button("Cancel", ImVec2(120, 0))) {
							ImGui::CloseCurrentPopup();
							close = true;
						}
						ImGui::EndPopup();

						if (close) {
							ImGui::CloseCurrentPopup();
						}
					}

					ImGui::PopItemWidth();
					ImGui::PopFont();
					ImGui::EndPopup();
				}
			}
			else {
				static char tempName[64];

				if (firstContext) {
					strncpy_s(tempName, layer.name.c_str(), 64);
					firstContext = false;
				}

				ImGui::PushFont(font17);
				if (ImGui::BeginPopupContextItem()) {
					ImGui::Text("Edit name:");

					if (ImGui::InputText("##edit", tempName, IM_ARRAYSIZE(tempName), ImGuiInputTextFlags_EnterReturnsTrue)) {
						ImGui::CloseCurrentPopup();
						layer.name = tempName;
					}

					if (ImGui::Button("OK")) {
						ImGui::CloseCurrentPopup();
						layer.name = tempName;
					}
					ImGui::EndPopup();
				}
				else {
					renameLayer = false;
					firstContext = true;
				}
				ImGui::PopFont();
			}
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

			enum class ToolType toolType = ToolType::NONE;
			
			if (Navigator::GetInstance()->selectedTool) {
				toolType = Navigator::GetInstance()->selectedTool->GetType();
			}

			if (ImGui::Selectable("Selection mode", toolType == ToolType::SELECT)) {
				Navigator::GetInstance()->UseTool(ToolType::SELECT);
			}
			if (ImGui::Selectable("Line mode", toolType == ToolType::LINE)) {
				Navigator::GetInstance()->UseTool(ToolType::LINE);
			}
			if (ImGui::Selectable("Line strip mode", toolType == ToolType::LINE_STRIP)) {
				Navigator::GetInstance()->UseTool(ToolType::LINE_STRIP);
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
			auto nav = Navigator::GetInstance();

			char str[1024];
			snprintf(str, 1024, "Mouse: %.2f|%.2f Snap: %.2f|%.2f", nav->mousePosition.x,
				nav->mousePosition.y, nav->mouseSnapped.x, nav->mouseSnapped.y);
			ImGui::Text(str);

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
			enableProfiling = ENABLE_PROFILING;
			enableImGuiDemoWindow = true;
			enableImPlotDemoWindow = true;
		}

		~GuiLayer() {
		}

		void OnImGuiAttach() override {
			//ImGui::GetIO().IniFilename = NULL;	// Prevent ImGui from saving a .ini file

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

			// Handle LayerWindow events
			if (layers.addLayerFlag) {
				layers.addLayerFlag = false;
				Navigator::GetInstance()->AddLayer();
			}

			if (layers.moveLayerFrontFlag) {
				layers.moveLayerFrontFlag = false;
				Navigator::GetInstance()->file.MoveLayerFront(layers.moveLayerFrontID);
			}

			if (layers.moveLayerBackFlag) {
				layers.moveLayerBackFlag = false;
				Navigator::GetInstance()->file.MoveLayerBack(layers.moveLayerBackID);
			}

			if (layers.layerSelectedFlag) {
				layers.layerSelectedFlag = false;
				Navigator::GetInstance()->OnLayerSelected(layers.selectedLayer);
			}

			if (layers.deleteLayerFlag) {
				layers.deleteLayerFlag = false;
				LOG_TRACE("Deleting layer {}", layers.deleteLayer);
				Navigator::GetInstance()->file.RemoveLayer(layers.deleteLayer);
			}
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