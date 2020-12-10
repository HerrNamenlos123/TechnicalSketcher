#pragma once

#include "StaticImGuiWindow.h"
#include "config.h"
#include "LayerList.h"

#define __IMGUI_WINDOW_FLAGS (ImGuiWindowFlags_NoTitleBar | \
						      ImGuiWindowFlags_NoMove | \
					          ImGuiWindowFlags_NoResize | \
					          ImGuiWindowFlags_NoCollapse)


class GuiRibbonWindow : public StaticImGuiWindow<> {
public:

	// Width is not known yet and must be set in the update function
	GuiRibbonWindow() : StaticImGuiWindow("guiRibbonWindow", { 0, 0 }, { 0, GUI_RIBBON_HEIGHT }, __IMGUI_WINDOW_FLAGS) {

	}

	// Core logic in here
	void updateWindow() override {

		

	}

	void update(glm::vec2 osWindowSize) {

		// Set window size to adjust when the os window is resized
		windowSize.x = osWindowSize.x;

		// Call the main update function, which in turn calls above updateWindow()
		updateStaticImGuiWindow();
	}

};





class GuiLayerWindow : public StaticImGuiWindow<LayerList&> {

	bool wasMouseOnWindow = false;

public:

	bool addLayerFlag = false;
	bool layerSelectedFlag = false;
	bool moveLayerFrontFlag = false;
	bool moveLayerBackFlag = false;
	LayerID selectedLayer = -1;
	LayerID moveLayerBackID = -1;
	LayerID moveLayerFrontID = -1;
	bool mouseEnteredWindowFlag = false;

	GuiLayerWindow() : StaticImGuiWindow("guiLayerWindow", { 0, GUI_RIBBON_HEIGHT }, 
										 { GUI_LEFT_BAR_WIDTH, GUI_LAYER_WINDOW_HEIGHT }, 
										 __IMGUI_WINDOW_FLAGS) {

	}

	// Core logic in here
	void updateWindow(LayerList& layers) override {

		ImGui::PushFont(font_pt20);

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

		for (LayerID id : layers.getSortedLayerIDs()) {
			Layer& layer = layers.findLayer(id);

			std::string name = layer.name + "##" + std::to_string(layer.layerID);

			// Draw GUI element and if clicked save flag, which will be read from the main loop event handler
			if (ImGui::Selectable(name.c_str(), layers.getSelectedLayerID() == layer.layerID)) {
				selectedLayer = layer.layerID;
				layerSelectedFlag = true;
			}

			// Remember if any selectable is hovered
			if (ImGui::IsItemHovered())
				anyActive = true;

			// Drag and drop
			if (ImGui::IsItemActive() && !ImGui::IsItemHovered() && isMouseOnWindow) {
				if (ImGui::GetMouseDragDelta(0).y < 0.f)
				{
					moveLayerFrontID = layer.layerID;
					moveLayerFrontFlag = true;
					ImGui::ResetMouseDragDelta();
				}
				else {
					moveLayerBackID = layer.layerID;
					moveLayerBackFlag = true;
					ImGui::ResetMouseDragDelta();
				}
			}

			// Draw the preview of the layer
			if (ImGui::IsItemHovered()) {
				if (layer.bitmap != nullptr) {
					ImGui::BeginTooltip();
					ImGui::Image(layer.bitmap, ImVec2(GUI_PREVIEWWINDOW_SIZE, GUI_PREVIEWWINDOW_SIZE));
					ImGui::EndTooltip();
				}
				else {
					std::cout << "WARNING: Bitmap is nullptr!!!" << std::endl;
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

	void update(LayerList& layerlist) {

		// Call the main update function, which in turn calls above updateWindow()
		updateStaticImGuiWindow(layerlist);
	}
};






class GuiToolboxWindow : public StaticImGuiWindow<> {
public:

	bool changeToolFlag = false;
	enum CursorTool clickedTool = TOOL_SELECT;
	enum CursorTool selectedTool = TOOL_SELECT;

	GuiToolboxWindow() : StaticImGuiWindow("guiToolboxWindow", { 0, GUI_RIBBON_HEIGHT + GUI_LAYER_WINDOW_HEIGHT }, 
										   { GUI_LEFT_BAR_WIDTH, GUI_TOOLBOX_WINDOW_HEIGHT }, 
										   __IMGUI_WINDOW_FLAGS | ImGuiWindowFlags_NoBringToFrontOnFocus) {

	}

	// Core logic in here
	void updateWindow() override {

		ImGui::PushFont(font_pt16);

		if (ImGui::Selectable("Selection mode", selectedTool == TOOL_SELECT)) {
			clickedTool = TOOL_SELECT;
			changeToolFlag = true;
		}
		if (ImGui::Selectable("Line mode", selectedTool == TOOL_LINE)) {
			clickedTool = TOOL_LINE;
			changeToolFlag = true;
		}
		if (ImGui::Selectable("Line strip mode", selectedTool == TOOL_LINE_STRIP)) {
			clickedTool = TOOL_LINE_STRIP;
			changeToolFlag = true;
		}

		ImGui::PopFont();
	}

	void update() {

		// Call the main update function, which in turn calls above updateWindow()
		updateStaticImGuiWindow();
	}

};







class GuiMouseInfoWindow : public StaticImGuiWindow<> {

	glm::vec2 mousePos = { 0, 0 };
	glm::vec2 mouseSnapped = { 0, 0 };

public:

	// Height is not known yet and must be set in the update function
	GuiMouseInfoWindow() : StaticImGuiWindow("mouseInfoWindow", { 0, 0 }, 
											 { GUI_LEFT_BAR_WIDTH, GUI_MOUSEINFO_WINDOW_HEIGHT }, 
											 __IMGUI_WINDOW_FLAGS) {

	}

	// Core logic in here
	void updateWindow() override {

		ImGui::PushFont(font_pt11);

		std::stringstream text;
		text << std::fixed << std::setprecision(2) << "Mouse: ";
		if (mousePos.x >= 0)
			text << " ";
		text << mousePos.x << "|";
		if (mousePos.y >= 0)
			text << " ";
		text << mousePos.y << " Snap: " << std::setprecision(0);
		if (mouseSnapped.x >= 0)
			text << " ";
		text << mouseSnapped.x << "|";
		if (mouseSnapped.y >= 0)
			text << " ";
		text << mouseSnapped.y;
		ImGui::Text(text.str().c_str());

		ImGui::PopFont();
	}

	void update(glm::vec2 osWindowSize, glm::vec2 mouse_workspace, glm::vec2 mouseSnapped_workspace) {

		windowPosition.y = osWindowSize.y - GUI_MOUSEINFO_WINDOW_HEIGHT;
		mousePos = mouse_workspace;
		mouseSnapped = mouseSnapped_workspace;

		// Call the main update function, which in turn calls above updateWindow()
		updateStaticImGuiWindow();
	}

};
