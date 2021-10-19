#pragma once

#include "pch.h"
#include "config.h"
#include "Navigator.h"
#include "Tools/GenericTool.h"
#include "GuiTheme.h"
#include "Fonts/FontLoader.h"

//#define SPLASHSCREEN_FONT	"C:\\Windows\\Fonts\\consola.ttf"
//#define SPLASHSCREEN_FONT_SIZE	14

#define GUI_RIBBON_HEIGHT 100
#define GUI_LEFT_BAR_WIDTH 180
#define GUI_LAYER_WINDOW_HEIGHT 200
#define GUI_TOOLBOX_WINDOW_HEIGHT 300
#define GUI_MOUSEINFO_WINDOW_HEIGHT 25
#define GUI_PROPERTIES_WINDOW_WIDTH 200
#define GUI_PROPERTIES_WINDOW_HEIGHT 200

struct FontContainer : public Battery::FontContainer {
	const ImWchar icons_ranges[3] = { 0xe005, 0xf8ff, 0 };
	ImFont* sansFont9 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_OPENSANS, 9);
	ImFont* sansFont14 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_OPENSANS, 14);
	ImFont* sansFont17 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_OPENSANS, 17);
	ImFont* sansFont22 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_OPENSANS, 22);
	ImFont* materialFont35 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_MATERIAL_ICONS, 35, nullptr, icons_ranges);
	ImFont* materialFont22 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_MATERIAL_ICONS, 22, nullptr, icons_ranges);
	ImFont* segoeFont22 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_SEGOE_MDL2, 22, nullptr, icons_ranges);
};

//struct FontContainer : public Battery::FontContainer {
//	const ImWchar icons_ranges[3] = { 0xe005, 0xf8ff, 0 };
//	ImFont* sansFont9		= defaultFont;
//	ImFont* sansFont14		= defaultFont;
//	ImFont* sansFont17		= defaultFont;
//	ImFont* sansFont22		= defaultFont;
//	ImFont* materialFont35	= defaultFont;
//	ImFont* materialFont22	= defaultFont;
//	ImFont* segoeFont22		= defaultFont;
//};

static void HelpMarker(const char* desc, ImFont* font)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushFont(font);
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::PopFont();
		ImGui::EndTooltip();
	}
}

static void ToolTip(const char* msg, ImFont* font) {
	if (ImGui::IsItemHovered()) {
		ImGui::PushFont(font);
		ImGui::BeginTooltip();
		ImGui::Text(msg);
		ImGui::EndTooltip();
		ImGui::PopFont();
	}
}

class RibbonWindow : public Battery::ImGuiPanel<> {

	std::string fileToOpen = "";
	int snap = 0;
	bool openOptions = false;

public:
	RibbonWindow() : Battery::ImGuiPanel<>("RibbonWindow", { 0, 0 }, { 0, 0 }, DEFAULT_IMGUI_PANEL_FLAGS | 
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse) {
	}

	void OnAttach() {

	}

	void OnDetach() {

	}

	void OnUpdate() override {
		size.x = Battery::GetMainWindow().GetWidth();
		size.y = GUI_RIBBON_HEIGHT;

		if (fileToOpen != "") {
			Navigator::GetInstance()->OpenFile(fileToOpen);
			fileToOpen = "";
		}
	}

	void MenuTab() {

		ImGui::MenuItem("TechnicalSketcher", NULL, false, false);
		if (ImGui::MenuItem("New File", "Ctrl+N")) {
			Navigator::GetInstance()->OpenEmptyFile();
		}
		if (ImGui::MenuItem("New Window", "Ctrl+Shift+N")) {
			Navigator::GetInstance()->StartNewApplicationInstance();
		}
		if (ImGui::MenuItem("Open", "Ctrl+O")) {
			Navigator::GetInstance()->OpenFile();
		}
		if (ImGui::BeginMenu("Open Recent")) {
			std::vector<std::string> recentFiles = Navigator::GetInstance()->GetRecentFiles();

			if (recentFiles.size() == 0) {
				ImGui::MenuItem("No recent files", NULL, false, false);
			}
			else {
				// Loop in reverse order, so the most recent is on top
				for (size_t i = recentFiles.size() - 1; i < recentFiles.size(); i--) {
					std::string& file = recentFiles[i];

					if (ImGui::MenuItem(file.c_str())) {
						fileToOpen = file;
					}
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Save", "Ctrl+S")) {
			Navigator::GetInstance()->SaveFile();
		}
		if (ImGui::MenuItem("Save As..", "Ctrl+Shift+S")) {
			Navigator::GetInstance()->SaveFileAs();
		}

		ImGui::Separator();
		if (ImGui::MenuItem("Options")) {
			openOptions = true;
		}

		if (ImGui::MenuItem("Quit", "Alt+F4")) {
			Navigator::GetInstance()->CloseApplication();
		}
	}

	void OptionsPopup() {

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
				Battery::TimeUtils::Sleep(0.03);
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

	void EditTab() {
		if (ImGui::MenuItem("Undo", "CTRL+Z")) {
			Navigator::GetInstance()->UndoAction();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Cut", "CTRL+X")) {
			Navigator::GetInstance()->CutClipboard();
		}
		if (ImGui::MenuItem("Copy", "CTRL+C")) {
			Navigator::GetInstance()->CopyClipboard();
		}
		if (ImGui::MenuItem("Paste", "CTRL+V")) {
			Navigator::GetInstance()->PasteClipboard();
		}
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

		OptionsPopup();
	}

	bool IsItemActiveLastFrame() {
		ImGuiContext& g = *GImGui;
		if (g.ActiveIdPreviousFrame)
			return g.ActiveIdPreviousFrame == g.CurrentWindow->DC.LastItemId;
		return false;
	}

	bool IsItemReleased() {
		return IsItemActiveLastFrame() && !ImGui::IsItemActive();
	}

	void MainButtons() {

		Navigator::GetInstance()->popupExportOpen = false;

		ImGui::PushFont(GetFontContainer<FontContainer>()->segoeFont22);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

		if (ImGui::Button("\uE7C3##NewFile")) {
			Navigator::GetInstance()->OpenEmptyFile();
		}
		ToolTip("Create an empty file", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();

		if (ImGui::Button("\uEC50##Open")) {
			Navigator::GetInstance()->OpenFile();
		}
		ToolTip("Open an existing file", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();

		if (ImGui::Button("\uE74E##Save")) {
			Navigator::GetInstance()->SaveFile();
		}
		ToolTip("Save the current file", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();

		if (ImGui::Button("\uE792##SaveAs")) {
			Navigator::GetInstance()->SaveFileAs();
		}
		ToolTip("Save the file with a new name", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();

		if (ImGui::Button("\uE77F##Export")) {
			ImGui::OpenPopup("Export rendering");
		}
		ToolTip("Copy a rendering of the file to the clipboard", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 35);

		if (ImGui::Button("\uE7A7##Undo")) {
			Navigator::GetInstance()->UndoAction();
		}
		ToolTip("Undo the last action", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();




		// Export popup
		bool exp = false;
		ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont17);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
		if (ImGui::BeginPopupModal("Export rendering", NULL, 
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {

			Navigator::GetInstance()->popupExportOpen = true;

			ImGui::Checkbox("Transparent background", &Navigator::GetInstance()->exportTransparent);
			ImGui::Text("Don't choose this for MathCAD");
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
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

			Navigator::GetInstance()->exportDPI = std::min(std::max(Navigator::GetInstance()->exportDPI, 50.f), 1000.f);

			if (ImGui::Button("OK", ImVec2(120, 0))) {	// Delete the layer now
				ImGui::CloseCurrentPopup();
				exp = true;
			}
			ToolTip("The image will be copied directly to your clipboard", GetFontContainer<FontContainer>()->sansFont17);
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		ImGui::PopFont();

		if (exp) {
			Navigator::GetInstance()->ExportClipboardRendering();
		}


		ImGui::SetCursorPosX(Battery::GetMainWindow().GetWidth() - 600);



		if (ImGui::Button("\uE7AD##ResetViewport")) {
			Navigator::GetInstance()->ResetViewport();
		}
		ToolTip("Reset the camera to the origin", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();


		bool pop = false;
		if (!Navigator::GetInstance()->gridShown) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			pop = true;
		}
		if (ImGui::Button("\uE80A##ToggleGrid")) {
			Navigator::GetInstance()->gridShown = !Navigator::GetInstance()->gridShown;
		}
		ToolTip("Show or hide the grid", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();
		if (pop) {
			ImGui::PopStyleColor();
		}


		pop = false;
		bool pop2 = false;
		if (Navigator::GetInstance()->infiniteSheet || !Navigator::GetInstance()->gridShown) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			pop = true;
		}
		if (!Navigator::GetInstance()->gridShown) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			pop2 = true;
		}
		if (ImGui::Button("\uE14E##ToggleInfiniteGrid")) {
			Navigator::GetInstance()->infiniteSheet = !Navigator::GetInstance()->infiniteSheet;
		}
		ToolTip("Toggle A4/infinite grid", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();
		if (pop) {
			ImGui::PopStyleColor();
		}
		if (pop2) {
			ImGui::PopItemFlag();
		}

		ImGui::PopStyleVar();

		// Background color
		ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont17);
		ImGui::Text("Background");
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

		glm::vec4 color = Navigator::GetInstance()->file.backgroundColor;
		color /= 255;
		ImGui::ColorEdit4("##BackgroundColor", (float*)&color.x, ImGuiColorEditFlags_NoInputs);
		color *= 255;

		if (color != Navigator::GetInstance()->file.backgroundColor) {
			Navigator::GetInstance()->file.FileChanged();
			Navigator::GetInstance()->file.backgroundColor = color;
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10);
		ImGui::PopFont();

		ImGui::SameLine();




		// SnapSize slider
		ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont17);

		float xpos = 660;
		float ypos = 15;
		ImGui::SetCursorPosX(Battery::GetMainWindow().GetWidth() - 1000 + xpos);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 25 + ypos);
		ImGui::Text("Snap size");
		ImGui::SameLine();

		ImGui::PushItemWidth(150);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7);
		int oldSnap = snap;
		ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SliderInt("##SnapSizeSlider", &snap, -4, 4, "");
		ImGui::SameLine();
		ImGui::PopFont();

		ImGui::PushItemWidth(50);
		ImGui::InputFloat("##SnapSizeField", &Navigator::GetInstance()->snapSize, 0, 0, "%.03f");
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 7);
		ImGui::Text("mm");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PopFont();
		ImGui::PopFont();

		//ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 155);

		if (snap != oldSnap) {	// Slider was changed
			Navigator::GetInstance()->snapSize = Navigator::GetInstance()->defaultSnapSize * pow(2, snap);
		}

		Navigator::GetInstance()->snapSize = std::max(Navigator::GetInstance()->snapSize, 0.125f);

		ImGui::PopItemWidth();
		
	}

	void OnRender() override {

		ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont17);

		// Show the main menu bar
		MenuBar();

		// Show all main buttons
		MainButtons();

		ImGui::PopFont();

	}
};

class LayerWindow : public Battery::ImGuiPanel<> {

	bool wasMouseOnWindow = false;	// For the mouse enter event
	bool firstContext = true;
	Layer newLayer;
	bool duplicateLayer = false;

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

	LayerWindow() : ImGuiPanel<>("LayerWindow", { 0, GUI_RIBBON_HEIGHT }, { GUI_LEFT_BAR_WIDTH, GUI_LAYER_WINDOW_HEIGHT }, 
		DEFAULT_IMGUI_PANEL_FLAGS | ImGuiWindowFlags_NoBringToFrontOnFocus), newLayer(std::string("")) {
	}

	void OnAttach() {
	}

	void OnDetach() {
		// Font does not need to be deleted
	}

	void OnUpdate() override {
		if (duplicateLayer) {
			duplicateLayer = false;
			Navigator::GetInstance()->file.PushLayer(std::move(newLayer));
		}
	}

	void OnRender() override {

		if (!wasMouseOnWindow && mouseOnPanel) {
			Navigator::GetInstance()->file.GeneratePreviews();
		}

		ImGui::PushFont(GetFontContainer<FontContainer>()->segoeFont22);
		ImGui::Text("\uE81E"); ImGui::SameLine();
		ImGui::PopFont();
		ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont22);
		ImGui::Text("Layers"); ImGui::SameLine();
		HelpMarker("Drag the layers up or down to change the order", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SameLine();

		// Add button now
		ImGui::SetCursorPosX(GUI_LEFT_BAR_WIDTH * 0.75);
		
		// Set flag, which will be read and acted on in the main loop event handler
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushItemWidth(30);
		ImGui::PushFont(GetFontContainer<FontContainer>()->materialFont22);
		addLayerFlag = ImGui::Button("\uE02E##AddLayer", ImVec2(35, 25));
		ImGui::PopFont();
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::Separator();

		float fontHeight = 22;					// Font height
		float itemHeight = fontHeight + 4;		// Font height + 4 pixels
		ImGuiIO& io = ImGui::GetIO();

		bool anyActive = false;

		SketchFile* file = &Navigator::GetInstance()->file;		// Get a reference to the current file
		Navigator::GetInstance()->popupDeleteLayerOpen = false;

		for (const auto& layer : file->GetLayers()) {
		
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
			if (ImGui::IsItemActive() && !ImGui::IsItemHovered() && mouseOnPanel) {
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

		if (mouseOnPanel && !wasMouseOnWindow) {
			mouseEnteredWindowFlag = true;
		}
		wasMouseOnWindow = mouseOnPanel;

		ImGui::PopFont();
	}

	void ContextMenu(const Layer& layer) {

		if (ImGui::BeginPopupContextItem()) {

			ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont17);
			float buttonWidth = 80;

			// Duplicate layer
			char duplicate[32];
			snprintf(duplicate, 32, "Duplicate##%zu", (size_t)layer.GetID());
			if (ImGui::Button(duplicate, ImVec2(buttonWidth, 0))) {
				// Duplicate the layer now
				newLayer = Navigator::GetInstance()->file.DuplicateActiveLayer();
				duplicateLayer = true;
				ImGui::CloseCurrentPopup();
			}

			// Rename layer
			char rename[32];
			snprintf(rename, 32, "Rename##%zu", (size_t)layer.GetID());
			if (ImGui::Button(rename, ImVec2(buttonWidth, 0))) {
				ImGui::OpenPopup("Rename Layer");
			}

			// Delete layer
			char remove[32];
			snprintf(remove, 32, "Delete##%zu", (size_t)layer.GetID());
			if (ImGui::Button(remove, ImVec2(buttonWidth, 0))) {
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

				Navigator::GetInstance()->popupDeleteLayerOpen = true;
			}

			if (ImGui::BeginPopup("Rename Layer")) {
				static char tempName[64];

				if (firstContext) {
					strncpy_s(tempName, layer.name.c_str(), 64);
					firstContext = false;
				}

				ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont17);
				ImGui::Text("Edit name:");

				if (ImGui::InputText("##edit", tempName, IM_ARRAYSIZE(tempName), ImGuiInputTextFlags_EnterReturnsTrue)) {
					ImGui::CloseCurrentPopup();
					Navigator::GetInstance()->file.SetLayerName(layer.GetID(), tempName);
				}

				if (ImGui::Button("OK")) {
					ImGui::CloseCurrentPopup();
					Navigator::GetInstance()->file.SetLayerName(layer.GetID(), tempName);
				}
				ImGui::PopFont();
				ImGui::EndPopup();
			}
			else {
				firstContext = true;
			}

			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}
};

class ToolboxWindow : public Battery::ImGuiPanel<> {
public:

	ToolboxWindow() : ImGuiPanel<>("ToolboxWindow", { 0, GUI_RIBBON_HEIGHT + GUI_LAYER_WINDOW_HEIGHT }, { GUI_LEFT_BAR_WIDTH, GUI_TOOLBOX_WINDOW_HEIGHT }, 
		DEFAULT_IMGUI_PANEL_FLAGS | ImGuiWindowFlags_NoBringToFrontOnFocus) {
	}

	void OnAttach() {
	}

	void OnDetach() {
		// Font does not need to be deleted
	}

	void OnUpdate() override {
	}

	void OnRender() override {

		ImGui::PushFont(GetFontContainer<FontContainer>()->materialFont35);
		float buttonDistance = 8;
		ImVec2 buttonSize = { ImGui::GetFont()->FontSize, ImGui::GetFont()->FontSize };

		enum class ToolType toolType = ToolType::NONE;
		if (Navigator::GetInstance()->selectedTool) {
			toolType = Navigator::GetInstance()->selectedTool->GetType();
		}
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonDistance / 2);

		// Selection tool
		ImVec2 pos1 = ImGui::GetCursorPos();
		if (ImGui::Selectable("\uEF52##SelectionTool", toolType == ToolType::SELECT, 0, buttonSize)) {
			Navigator::GetInstance()->UseTool(ToolType::SELECT);
		}
		ToolTip("Selection tool", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SetCursorPos(pos1);
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonDistance);

		// Line tool
		ImVec2 pos2 = ImGui::GetCursorPos();
		if (ImGui::Selectable("\uF108##LineTool", toolType == ToolType::LINE, 0, buttonSize)) {
			Navigator::GetInstance()->UseTool(ToolType::LINE);
		}
		ToolTip("Line tool", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SetCursorPos(pos2);
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonDistance);

		// Line strip tool
		ImVec2 pos3 = ImGui::GetCursorPos();
		if (ImGui::Selectable("\uE922##LineStripTool", toolType == ToolType::LINE_STRIP, 0, buttonSize)) {
			Navigator::GetInstance()->UseTool(ToolType::LINE_STRIP);
		}
		ToolTip("Line strip tool", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SetCursorPos(pos3);
		ImGui::SameLine();
		ImGui::SetCursorPosX(pos1.x);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + buttonDistance * 2 + buttonSize.y);

		// Circle tool
		ImVec2 pos4 = ImGui::GetCursorPos();
		if (ImGui::Selectable("\uE3FA##CircleTool", toolType == ToolType::CIRCLE, 0, buttonSize)) {
			Navigator::GetInstance()->UseTool(ToolType::CIRCLE);
		}
		ToolTip("Circle tool", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SetCursorPos(pos4);
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonDistance);

		// Arc tool
		ImVec2 pos5 = ImGui::GetCursorPos();
		if (ImGui::Selectable("\uE3FC##ArcTool", toolType == ToolType::ARC, 0, buttonSize)) {
			Navigator::GetInstance()->UseTool(ToolType::ARC);
		}
		ToolTip("Arc tool", GetFontContainer<FontContainer>()->sansFont17);
		ImGui::SetCursorPos(pos5);
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonDistance);

		ImGui::PopFont();
	}
};

class PropertiesWindow : public Battery::ImGuiPanel<> {
public:

	PropertiesWindow() : ImGuiPanel<>("PropertiesWindow", { 0, GUI_RIBBON_HEIGHT },
		{ GUI_PROPERTIES_WINDOW_WIDTH, GUI_PROPERTIES_WINDOW_HEIGHT }) {
	}

	void OnAttach() {
	}

	void OnDetach() {
		// Font does not need to be deleted
	}

	void OnUpdate() override {
		position.x = Battery::GetMainWindow().GetWidth() - GUI_PROPERTIES_WINDOW_WIDTH;
	}

	void OnRender() override {

		ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont17);

		if (Navigator::GetInstance()->selectedTool) {
			Navigator::GetInstance()->selectedTool->ShowPropertiesWindow();
		}

		ImGui::PopFont();
	}
};

class MouseInfoWindow : public Battery::ImGuiPanel<> {
public:

	MouseInfoWindow() : ImGuiPanel<>("MouseInfoWindow", { 0, 0 }, 
		{ GUI_LEFT_BAR_WIDTH, GUI_MOUSEINFO_WINDOW_HEIGHT }) {
	}

	void OnAttach() {
	}

	void OnDetach() {
		// Font does not need to be deleted
	}

	void OnUpdate() override {
		position.y = Battery::GetMainWindow().GetHeight() - GUI_MOUSEINFO_WINDOW_HEIGHT;
	}

	void OnRender() override {

		ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont9);
		auto nav = Navigator::GetInstance();

		char str[1024];
		snprintf(str, 1024, "Mouse: %.2f|%.2f Snap: %.2f|%.2f", nav->mousePosition.x,
			nav->mousePosition.y, nav->mouseSnapped.x, nav->mouseSnapped.y);
		ImGui::Text(str);

		ImGui::PopFont();
	}
};

class UserInterface : public Battery::ImGuiLayer<FontContainer> {
public:

	RibbonWindow ribbon;
	LayerWindow layers;
	ToolboxWindow toolbox;
	MouseInfoWindow mouseInfo;
	PropertiesWindow propertiesWindow;
	bool wasPropertiesWindowShown = false;

	ALLEGRO_SYSTEM_MOUSE_CURSOR mouseCursor = ALLEGRO_SYSTEM_MOUSE_CURSOR_NONE;

	UserInterface() : Battery::ImGuiLayer<FontContainer>("UserInterface") {}

	void OnImGuiAttach() override {
		SetImGuiTheme();
		
		if (Navigator::GetInstance()->imguiFileLocation != "")
			ImGui::GetIO().IniFilename = Navigator::GetInstance()->imguiFileLocation.c_str();
		else
			ImGui::GetIO().IniFilename = NULL;
		
		ribbon.OnAttach();
		layers.OnAttach();
		toolbox.OnAttach();
		mouseInfo.OnAttach();
		propertiesWindow.OnAttach();
	}

	void OnImGuiDetach() override {

		ribbon.OnDetach();
		layers.OnDetach();
		toolbox.OnDetach();
		mouseInfo.OnDetach();
		propertiesWindow.OnDetach();
	}

	void OnImGuiUpdate() override {
		ribbon.Update();
		layers.Update();
		toolbox.Update();
		mouseInfo.Update();
		propertiesWindow.Update();
	}

	void OnImGuiRender() override {
		ribbon.Render();
		layers.Render();
		toolbox.Render();
		mouseInfo.Render();
		
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

	void DrawTabInfoBox() {
		if (Navigator::GetInstance()->tabbedShapeInfo) {
			if (Navigator::GetInstance()->selectedTool) {
				ShapeID hovered = static_cast<SelectionTool*>
					(Navigator::GetInstance()->selectedTool)->selectionHandler.GetLastHoveredShape();
				auto opt = Navigator::GetInstance()->file.FindShape(hovered);
		
				if (hovered != -1 && opt.has_value()) {
					ImGui::PushFont(GetFontContainer<FontContainer>()->sansFont14);
					ImGui::BeginTooltip();
					ImGui::Text("Shape #%d, Type: %s", opt.value().get().GetID(), opt.value().get().GetTypeString().c_str());
					ImGui::EndTooltip();
					ImGui::PopFont();
				}
			}
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
