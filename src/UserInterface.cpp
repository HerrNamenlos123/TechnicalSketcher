
#include "pch.h"
#include "UserInterface.h"
#include "Battery/AllegroDeps.h"

UserInterface::UserInterface() : Battery::ImGuiLayer<FontContainer>("UserInterface") {
	mouseCursor = ALLEGRO_SYSTEM_MOUSE_CURSOR_NONE;
}

void RibbonWindow::OptionsPopup() {

	if (openOptions) {
		ImGui::OpenPopup("Options##Window");
		openOptions = false;
	}

	bool exp = false;
	Navigator::GetInstance()->popupSettingsOpen = false;
	ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont17);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
	if (ImGui::BeginPopupModal("Options##Window", NULL,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {

		Navigator::GetInstance()->popupSettingsOpen = true;

		ImGui::Checkbox("Transparent background", &Navigator::GetInstance()->exportTransparent);
		ToolTip("Un-tick this for MathCAD", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::Separator();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
		ImGui::Text("DPI");
		ImGui::SameLine();
		ImGui::PushItemWidth(210);
		ImGui::InputFloat("##DPIField", &Navigator::GetInstance()->exportDPI);
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(240);
		ImGui::SliderFloat("##DPISlider", &Navigator::GetInstance()->exportDPI, 50, 1000);
		ImGui::PopItemWidth();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
		ImGui::Separator();

		if (ImGui::Button("Reset User interface")) {
			Battery::GetMainWindow().SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_BUSY);
			Navigator::GetInstance()->ResetGui();
			Battery::Sleep(0.03);
			Battery::GetMainWindow().SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
		}
		ToolTip("Reset the layout of the user interface, if something gets lost", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::Separator();

		ImGui::Checkbox("Keep Application up-to-date", &Navigator::GetInstance()->keepUpToDate);
		ToolTip("If ticked, the application will receive fully automatic updates", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::Separator();

		if (ImGui::Button("Close", ImVec2(240, 0))) {
			ImGui::CloseCurrentPopup();
			Navigator::GetInstance()->SaveSettings();
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopFont();
}



void UserInterface::OnImGuiRender() {
	ribbon.Render();
	layers.Render();
	toolbox.Render();
	mouseInfo.Render();
	updateInfoWindow.Render();

	//ImGui::ShowStyleEditor();

	bool wasShownBefore = wasPropertiesWindowShown;
	wasPropertiesWindowShown = false;
	if (Navigator::GetInstance()->selectedTool) {
		if (Navigator::GetInstance()->selectedTool->IsPropertiesWindowShown()) {
			propertiesWindow.Render();
			wasPropertiesWindowShown = true;
		}
	}

	if (wasPropertiesWindowShown && !wasShownBefore) {	// Properties window is being opened, save state
		Navigator::GetInstance()->file.SaveActiveLayerState();
	}

	DrawTabInfoBox();

	if (mouseCursor != ALLEGRO_SYSTEM_MOUSE_CURSOR_NONE)
		Battery::GetMainWindow().SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_BUSY);

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
