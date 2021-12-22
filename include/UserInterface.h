#pragma once

#include "pch.h"
#include "config.h"
#include "Navigator.h"
#include "Tools/GenericTool.h"
#include "GuiTheme.h"
#include "Fonts/FontLoader.h"
#include "Fonts/Fonts.h"

//#define SPLASHSCREEN_FONT	"C:\\Windows\\Fonts\\consola.ttf"
//#define SPLASHSCREEN_FONT_SIZE	14

// TODO: Fix user interface with new Battery panels

// TODO: Print version on splash screen

#define GUI_RIBBON_HEIGHT 100
#define GUI_LEFT_BAR_WIDTH 180
#define GUI_LAYER_WINDOW_HEIGHT 200
#define GUI_TOOLBOX_WINDOW_HEIGHT 300
#define GUI_MOUSEINFO_WINDOW_HEIGHT 25
#define GUI_PROPERTIES_WINDOW_WIDTH 200
#define GUI_PROPERTIES_WINDOW_HEIGHT 200

#define GUI_UPDATEINFO_WIDTH 260
#define GUI_UPDATEINFO_HEIGHT 90
#define POPUP_TIME 8

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

static void LoadingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = size_arg;
	size.x -= style.FramePadding.x * 2;

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return;

	// Render
	window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_col);
	window->DrawList->AddRectFilled(bb.Min, ImVec2(bb.Min.x + size.x * value, bb.Max.y), fg_col);
}

static void Spinner(const char* label, double radius, int thickness, const ImU32& color) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return;

	// Render
	window->DrawList->PathClear();

	int num_segments = 30;
	int start = abs(std::sin(Battery::GetRuntime() * 1.8) * (num_segments - 5.0));

	double a_min = IM_PI * 2.0 * ((double)start) / (float)num_segments;
	double a_max = IM_PI * 2.0 * ((double)num_segments - 3) / (float)num_segments;

	const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

	for (int i = 0; i < num_segments; i++) {
		double a = a_min + ((double)i / (double)num_segments) * (a_max - a_min);
		window->DrawList->PathLineTo(ImVec2(
			(double)centre.x + std::cos(a + Battery::GetRuntime() * 14.0) * radius,
			(double)centre.y + std::sin(a + Battery::GetRuntime() * 14.0) * radius));
	}

	window->DrawList->PathStroke(color, false, thickness);
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
				for (size_t i = recentFiles.size() - 1; i < recentFiles.size(); i--) {	// TODO: Make this loop cleaner
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

	void OptionsPopup();

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
				if (layer.previewImage) {
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

class UpdateInfoWindow : public Battery::ImGuiPanel<> {

	bool had = true;

public:

	UpdateInfoWindow() : ImGuiPanel<>("UpdateInfoWindow", { 0, 0 }, { 0, 0 }, 
		DEFAULT_IMGUI_PANEL_FLAGS | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar) {}

	void OnAttach() {}
	void OnDetach() {}

	void OnUpdate() override {
		size = { GUI_UPDATEINFO_WIDTH, GUI_UPDATEINFO_HEIGHT };

		if (Navigator::GetInstance()->updateStatus == UpdateStatus::NOTHING) {
			HideWindow();
		} 
		else if (Navigator::GetInstance()->updateStatus == UpdateStatus::DONE) {
			size.x = 200;
			size.y = 105;
			if (Battery::GetRuntime() > Navigator::GetInstance()->timeSincePopup + POPUP_TIME) {
				Navigator::GetInstance()->updateStatus = UpdateStatus::NOTHING;
			}
		}
		else if (Navigator::GetInstance()->updateStatus == UpdateStatus::FAILED) {
			size.x = 205;
			size.y = 65;
			if (Battery::GetRuntime() > Navigator::GetInstance()->timeSincePopup + POPUP_TIME) {
				Navigator::GetInstance()->updateStatus = UpdateStatus::NOTHING;
			}
		}

		position.x = Battery::GetMainWindow().GetWidth() - size.x;
		position.y = Battery::GetMainWindow().GetHeight() - size.y;
	}

	void OnRender() override {
		auto fonts = GetFontContainer<FontContainer>();
		ImGui::PushFont(fonts->sansFont22);

		std::string info = "";
		float progress = 0.f;
		switch (Navigator::GetInstance()->updateStatus) {

		case UpdateStatus::INITIALIZING:
			info = "Preparing for update";
			break;

		case UpdateStatus::DOWNLOADING:
			info = "Downloading update";
			progress = Navigator::GetInstance()->updateProgress;
			break;

		case UpdateStatus::EXTRACTING:
			info = "Extracting update";
			progress = 1.f;
			break;

		case UpdateStatus::INSTALLING:
			info = "Installing update";
			progress = 1.f;
			break;

		case UpdateStatus::DONE:
			info = "TechnicalSketcher was\nupdated successfully";
			break;

		case UpdateStatus::FAILED:
			info = "TechnicalSketcher could\nnot be updated :(";
			break;

		default:
			break;
		}

		const ImU32 col = 0xFF13A300;
		const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);

		if (Navigator::GetInstance()->updateStatus == UpdateStatus::DONE) {
			static bool t = false;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
			ImGui::Text(info.c_str());
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
			if (ImGui::Button("Restart now")) {
				Navigator::GetInstance()->timeSincePopup = Battery::GetRuntime();
				if (Navigator::GetInstance()->CloseApplication()) {
					// And restart the application
					auto ret = Battery::ExecuteShellCommandSilent("start " + Navigator::GetInstance()->restartExecutablePath, true);
					Navigator::GetInstance()->updateStatus = UpdateStatus::NOTHING;
					if (!ret.first) {	// New version failed to launch
						Battery::GetApp().shouldClose = false;		// TODO: Do this properly
					}
				}
			}
		}
		else if (Navigator::GetInstance()->updateStatus == UpdateStatus::FAILED) {
			ImGui::Text(info.c_str());
		} 
		else {
			ImGui::Text(info.c_str());
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
			Spinner("##spinner", 15, 6, col);
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
			LoadingBar("##loading_bar", progress, ImVec2(180, 16), bg, col);
		}

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
	UpdateInfoWindow updateInfoWindow;
	bool wasPropertiesWindowShown = false;

	int mouseCursor;

	UserInterface();

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
		updateInfoWindow.OnAttach();
	}

	void OnImGuiDetach() override {

		ribbon.OnDetach();
		layers.OnDetach();
		toolbox.OnDetach();
		mouseInfo.OnDetach();
		propertiesWindow.OnDetach();
		updateInfoWindow.OnDetach();
	}

	void OnImGuiUpdate() override {
		ribbon.Update();
		layers.Update();
		toolbox.Update();
		mouseInfo.Update();
		updateInfoWindow.Update();
		propertiesWindow.Update();
	}

	void OnImGuiRender() override;

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
