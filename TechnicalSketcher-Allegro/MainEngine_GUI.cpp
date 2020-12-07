
#include "pch.h"
#include "MainEngine.h"


void MainEngine::drawLayersWindow() {

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoNav;

	ImGui::SetNextWindowPos(ImVec2(0, gui_leftUpperViewportCorner.y));
	ImGui::SetNextWindowSize(ImVec2(gui_leftUpperViewportCorner.x, gui_layersBoxHeight));
	bool isOpen = true;
	ImGui::GetStyle().WindowRounding = 0.0f;

	ImGui::Begin("LeftBarLayers", &isOpen, window_flags);
	{

		if (!events_layerWindowWasHovered && ImGui::IsWindowHovered()) {
			events_layerWindowMouseEnteredEventFlag = true;
		}
		events_layerWindowWasHovered = ImGui::IsWindowHovered();

		ImGui::PushFont(font_pt16);
		ImGui::Text("Layers"); ImGui::SameLine();
		ImGui::SetCursorPosX(gui_leftUpperViewportCorner.x * 0.8);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		if (ImGui::Button("+##AddLayer", ImVec2(25, 17))) {
			addLayer();
		}
		ImGui::PopStyleVar();
		ImGui::Separator();
		//ImGui::Columns(2);

		/*int _moveLayerUp = -1;
		int _moveLayerDown = -1;
		for (size_t i = 0; i < layers.size(); i++) {
			if (ImGui::Selectable(layers[i].name.c_str(), selectedLayer == static_cast<int>(i))) {
				selectLayer(static_cast<int>(i));
			}

			// Draw the preview of the layer
			if (ImGui::IsItemHovered())
			{
				if (gui_previewChanged) {
					generateLayerPreviews();	// Flag is reset in there
				}

				if (layers[i].bitmap != nullptr) {
					ImGui::BeginTooltip();
					ImGui::Image(layers[i].bitmap, ImVec2(gui_hoverWindowSize.x, gui_hoverWindowSize.y));
					ImGui::EndTooltip();
				}
			}


			ImGui::PushFont(font_pt16);
			ImGui::NextColumn();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			if (i == 0) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			bool up = ImGui::Button(("^##" + layers[i].name).c_str(), ImVec2(25, 17)); ImGui::SameLine();
			if (i == 0) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
			if (i == layers.size() - 1) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			bool down = ImGui::Button(("v##" + layers[i].name).c_str(), ImVec2(25, 17));
			if (i == layers.size() - 1) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			ImGui::PopStyleVar();
			ImGui::NextColumn();
			ImGui::PopFont();

			if (up) {
				_moveLayerUp = static_cast<int>(i);
			}
			if (down) {
				_moveLayerDown = static_cast<int>(i);
			}
		}
		ImGui::PopFont();

		if (_moveLayerUp != -1) {
			moveLayerUp(_moveLayerUp);
		}
		if (_moveLayerDown != -1) {
			moveLayerDown(_moveLayerDown);
		}*/

		layers.updateGUI();

		ImGui::PopFont();

	}
	ImGui::End();
}

void MainEngine::drawToolboxWindow() {

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoNav;

	ImGui::SetNextWindowPos(ImVec2(0, gui_leftUpperViewportCorner.y + gui_toolboxHeight));
	ImGui::SetNextWindowSize(ImVec2(gui_leftUpperViewportCorner.x, gui_toolboxHeight));
	bool isOpen = true;
	ImGui::GetStyle().WindowRounding = 0.0f;

	ImGui::Begin("ToolBox", &isOpen, window_flags);
	{
		ImGui::PushFont(font_pt16);
		if (ImGui::Selectable("Selection mode", cursorMode == MODE_SELECT)) {
			changeMode(MODE_SELECT);
		}
		if (ImGui::Selectable("Line mode", cursorMode == MODE_LINE)) {
			changeMode(MODE_LINE);
		}
		if (ImGui::Selectable("Line strip mode", cursorMode == MODE_LINE_STRIP)) {
			changeMode(MODE_LINE_STRIP);
		}
		ImGui::PopFont();
	}
	ImGui::End();
}

void MainEngine::drawMousePositionWindow() {

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoNav;
	//window_flags |= ImGuiWindowFlags_NoBackground;

	int h = 30;
	ImGui::SetNextWindowPos(ImVec2(0, height - h));
	ImGui::SetNextWindowSize(ImVec2(gui_leftUpperViewportCorner.x, h));
	bool isOpen = true;
	ImGui::GetStyle().WindowRounding = 0.0f;

	ImGui::Begin("MouseInfo", &isOpen, window_flags);
	{
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
	ImGui::End();
}
