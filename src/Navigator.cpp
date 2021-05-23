
#include "pch.h"
#include "Navigator.h"
#include "Layer.h"
#include "config.h"
#include "Application.h"

#include "Tools/SelectionTool.h"
#include "Tools/LineTool.h"
#include "Tools/LineStripTool.h"
#include "Tools/CircleTool.h"

void Navigator::CreateInstance() {
	instance.reset(new Navigator());
}

void Navigator::DestroyInstance() {
	instance.release();
}

Navigator* Navigator::GetInstance() {
	if (instance) {
		return instance.get();
	}

	throw Battery::Exception(__FUNCTION__"(): Can't return Navigator instance: Instance is nullptr!");
}

void Navigator::OnAttach() {
	UseTool(ToolType::SELECT);

	// Get the version of the application
	applicationVersion = GetApplicationVersion();

	// Register the clipboard format
	clipboardShapeFormat = GetClientApplication()->window.RegisterClipboardFormat(CLIPBOARD_FORMAT);

	// Load the location of the ImGui .ini file
	imguiFileLocation = GetSettingsDirectory() + IMGUI_FILENAME;
}

void Navigator::OnDetach() {
}

void Navigator::OnUpdate() {
	windowSize = glm::ivec2(GetClientApplication()->window.GetWidth(), GetClientApplication()->window.GetHeight());
	mousePosition = ConvertScreenToWorkspaceCoords(GetClientApplication()->window.GetMousePosition());
	mouseSnapped = round(mousePosition / snapSize) * snapSize;

	// Key control
	controlKeyPressed = GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL) || GetClientApplication()->GetKey(ALLEGRO_KEY_RCTRL);
	shiftKeyPressed = GetClientApplication()->GetKey(ALLEGRO_KEY_LSHIFT) || GetClientApplication()->GetKey(ALLEGRO_KEY_RSHIFT);
	
	// Allow smooth positioning when CTRL is pressed
	if (controlKeyPressed) {
		mouseSnapped = mousePosition;
	}

	// Update window title
	file.UpdateWindowTitle();
	
	// Handle all queued events
	UpdateEvents();

	// Print all shapes
	//PrintShapes();
}

void Navigator::OnRender() {

	ApplicationRenderer::BeginFrame();

	Battery::Renderer2D::DrawBackground(file.backgroundColor);

	// Draw first part of the selection box
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool)->RenderFirstPart();
		}
	}
	
	// Main elements of the application
	if (gridShown) {
		ApplicationRenderer::DrawGrid(infiniteSheet);
	}
	RenderShapes();
	
	// Draw shape preview
	if (selectedTool) {
		selectedTool->RenderPreview();
	}
	
	// Preview point
	if (previewPointShown) {
		ApplicationRenderer::DrawPreviewPoint(previewPointPosition);
	}

	// Draw second part of the selection box
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool)->RenderSecondPart();
		}
	}

	ApplicationRenderer::EndFrame();
}

void Navigator::OnEvent(Battery::Event* e) {
	switch (e->GetType()) {

	case Battery::EventType::MouseScrolled:
	{
		Battery::MouseScrolledEvent* event = static_cast<Battery::MouseScrolledEvent*>(e);
		scrollBuffer += event->dx + event->dy;
		e->SetHandled();
		break;
	}

	case Battery::EventType::MouseButtonPressed:
		mousePressedEventBuffer.push_back(*static_cast<Battery::MouseButtonPressedEvent*>(e));
		e->SetHandled();
		break;

	case Battery::EventType::MouseButtonReleased:
		mouseReleasedEventBuffer.push_back(*static_cast<Battery::MouseButtonReleasedEvent*>(e));
		e->SetHandled();
		break;

	case Battery::EventType::MouseMoved:
		mouseMovedEventBuffer.push_back(*static_cast<Battery::MouseMovedEvent*>(e));
		e->SetHandled();
		break;

	case Battery::EventType::KeyPressed:
		keyPressedEventBuffer.push_back(*static_cast<Battery::KeyPressedEvent*>(e));
		e->SetHandled();
		break;

	default:
		break;
	}
}






glm::vec2 Navigator::ConvertScreenToWorkspaceCoords(const glm::vec2& v) {
	return (v - panOffset - glm::vec2(GetClientApplication()->window.GetWidth(), 
		GetClientApplication()->window.GetHeight()) * 0.5f) / scale;
}

glm::vec2 Navigator::ConvertWorkspaceToScreenCoords(const glm::vec2& v) {
	return panOffset + v * scale + glm::vec2(GetClientApplication()->window.GetWidth(),
		GetClientApplication()->window.GetHeight()) * 0.5f;
}

float Navigator::ConvertWorkspaceToScreenDistance(float distance) {
	return distance * scale;
}

float Navigator::ConvertScreenToWorkspaceDistance(float distance) {
	return distance / scale;
}












void Navigator::MouseScrolled(float amount) {

	float scroll = amount * scrollFactor;
	float factor = 1 + std::abs(scroll);

	if (scroll > 0) {
		scale *= factor;
	}
	else {
		scale /= factor;
	}

	auto mPos = GetClientApplication()->window.GetMousePosition();
	glm::vec2 mouseToCenter = glm::vec2(panOffset.x - mPos.x + windowSize.x / 2.f,
										panOffset.y - mPos.y + windowSize.y / 2.f);

	if (scroll > 0)
		panOffset += mouseToCenter * factor - mouseToCenter;
	else
		panOffset -= mouseToCenter - mouseToCenter / factor;
}

void Navigator::UpdateEvents() {

	// Update the scroll events
	if (scrollBuffer != 0.f) {
		MouseScrolled(scrollBuffer);
		scrollBuffer = 0;
	}

	// Then update mouse pressed events
	for (Battery::MouseButtonPressedEvent event : mousePressedEventBuffer) {
		glm::vec2 position = ConvertScreenToWorkspaceCoords({ event.x, event.y });
		glm::vec2 snapped = round(position / snapSize) * snapSize;

		// Allow smooth positioning when CTRL is pressed
		if (controlKeyPressed) {
			snapped = position;
		}

		// Call all event functions
		bool left = event.button == 1;
		bool right = event.button == 2;
		bool wheel = event.button == 3;
		OnMouseClicked(position, snapped, left, right, wheel);
	}
	mousePressedEventBuffer.clear();

	// Next, mouse released events
	for (Battery::MouseButtonReleasedEvent event : mouseReleasedEventBuffer) {
		glm::vec2 position = ConvertScreenToWorkspaceCoords({ event.x, event.y });
		glm::vec2 snapped = round(position / snapSize) * snapSize;

		// Allow smooth positioning when CTRL is pressed
		if (controlKeyPressed) {
			snapped = position;
		}

		// Call all event functions
		bool left = event.button == 1;
		bool right = event.button == 2;
		bool wheel = event.button == 3;
		OnMouseReleased(position, left, right, wheel);
	}
	mouseReleasedEventBuffer.clear();

	// And finally mouse moved events
	for (Battery::MouseMovedEvent event : mouseMovedEventBuffer) {
		glm::vec2 position = ConvertScreenToWorkspaceCoords({ event.x, event.y });
		glm::vec2 snapped = round(position / snapSize) * snapSize;
		
		// Allow smooth positioning when CTRL is pressed
		if (controlKeyPressed) {
			snapped = position;
		}
		
		OnMouseMoved(position, snapped, event.dx, event.dy);
	}
	mouseMovedEventBuffer.clear();

	// Now, handle key events
	for (Battery::KeyPressedEvent event : keyPressedEventBuffer) {
		OnKeyPressed(&event);
	}
	keyPressedEventBuffer.clear();
}

void Navigator::CancelShape() {

	if (selectedTool) {
		selectedTool->CancelShape();
	}
	LOG_TRACE("Shape cancelled");

}

void Navigator::UseTool(enum class ToolType tool) {

	switch (tool) {

	case ToolType::NONE:
		LOG_WARN("Can't choose tool 'NONE'");
		selectedTool = &selectionTool;
		break;

	case ToolType::SELECT:
		selectedTool = &selectionTool;
		break;

	case ToolType::LINE:
		selectedTool = &lineTool;
		break;

	case ToolType::LINE_STRIP:
		selectedTool = &lineStripTool;
		break;

	case ToolType::CIRCLE:
		selectedTool = &circleTool;
		break;

	case ToolType::ARC:
		selectedTool = &arcTool;
		break;

	default:
		LOG_WARN("Unsupported tool type was selected");
		selectedTool = &selectionTool;
		break;
	}

	OnToolChanged();
}

void Navigator::PrintShapes() {
	
	// Print all shapes in the currently selected Layer
	LOG_WARN("Layer #{}: Name '{}'", file.GetActiveLayer().GetID(), file.GetActiveLayer().name);
	for (const auto& shape : file.GetActiveLayer().GetShapes()) {
		LOG_WARN("Shape #{}: ", shape->GetID());
		LOG_ERROR("Shape JSON Content: \n{}", shape->GetJson().dump(4));
	}
}

void Navigator::RemoveSelectedShapes() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool)->RemoveSelectedShapes();
		}
	}
}

void Navigator::MoveSelectedShapesLeft() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool)->MoveSelectedShapesLeft(snapSize);
		}
	}
}

void Navigator::MoveSelectedShapesRight() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool)->MoveSelectedShapesRight(snapSize);
		}
	}
}

void Navigator::MoveSelectedShapesUp() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool)->MoveSelectedShapesUp(snapSize);
		}
	}
}

void Navigator::MoveSelectedShapesDown() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool)->MoveSelectedShapesDown(snapSize);
		}
	}
}

void Navigator::SelectNextPossibleShape() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool)->SelectNextPossibleShape();
		}
	}
	tabbedShapeInfo = true;
}








void Navigator::OnKeyPressed(Battery::KeyPressedEvent* event) {
	
	switch (event->keycode) {

	case ALLEGRO_KEY_TAB:
		// Switch through all possibly selected shapes, is wrapped around automatically
		SelectNextPossibleShape();
		break;

	case ALLEGRO_KEY_DELETE:
		// Delete selected shapes
		RemoveSelectedShapes();
		break;

	case ALLEGRO_KEY_LEFT:
		// Move shape to the left by one unit
		MoveSelectedShapesLeft();
		break;

	case ALLEGRO_KEY_RIGHT:
		// Move shape to the right by one unit
		MoveSelectedShapesRight();
		break;

	case ALLEGRO_KEY_UP:
		// Move shape up by one unit
		MoveSelectedShapesUp();
		break;

	case ALLEGRO_KEY_DOWN:
		// Move shape down by one unit
		MoveSelectedShapesDown();
		break;

	case ALLEGRO_KEY_ESCAPE:
		// Reset tools
		OnEscapePressed();
		break;

	case ALLEGRO_KEY_Z:
		// Undo previous action
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			UndoAction();
		}
		break;

	case ALLEGRO_KEY_A:		// Select all
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			UseTool(ToolType::SELECT);
			SelectAll();
		}
		break;

	case ALLEGRO_KEY_O:		// Open
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			OpenFile();
		}
		break;

	case ALLEGRO_KEY_S:		// Save
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL) &&
			GetClientApplication()->GetKey(ALLEGRO_KEY_LSHIFT)) {
			SaveFileAs();
		}
		else if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {
			SaveFile();
		}
		break;

	case ALLEGRO_KEY_C:		// Copy
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			CopyClipboard();
		}
		break;

	case ALLEGRO_KEY_X:		// Cut
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			CutClipboard();
		}
		break;

	case ALLEGRO_KEY_V:		// Paste
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			UseTool(ToolType::SELECT);
			PasteClipboard();
		}
		break;

	case ALLEGRO_KEY_N:
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {
			if (GetClientApplication()->GetKey(ALLEGRO_KEY_LSHIFT)) {
				Navigator::GetInstance()->StartNewApplicationInstance();	// CTRL + SHIFT + N
			}
			else {
				Navigator::GetInstance()->OpenEmptyFile();		// CTRL + N
				UseTool(ToolType::SELECT);
			}
		}
		break;

	default:
		break;

	}
}

void Navigator::OnMouseClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel) {

	ShapeID shapeClicked = -1;

	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			shapeClicked = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetHoveredShape(position);
		}
	}

	if (shapeClicked == -1) {
		OnSpaceClicked(position, snapped, left, right, wheel);
	}
	else {
		OnShapeClicked(position, snapped, left, right, wheel, shapeClicked);
	}
}

void Navigator::OnMouseReleased(const glm::vec2& position, bool left, bool right, bool wheel) {
	if (selectedTool) {
		selectedTool->OnMouseReleased(position, left, right, wheel);
	}
}

void Navigator::OnMouseMoved(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {
	using namespace Battery;

	if (GetClientApplication()->window.GetLeftMouseButton() || 
		GetClientApplication()->window.GetRightMouseButton() || 
		GetClientApplication()->window.GetMouseWheel())
	{
		OnMouseDragged(position, snapped, dx, dy);
	}
	else {
		OnMouseHovered(position, snapped, dx, dy);
	}

	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			ShapeID hovered = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetLastHoveredShape();
			if (hovered == -1) {
				tabbedShapeInfo = false;
			}
		}
	}
}

void Navigator::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {
	if (selectedTool) {
		selectedTool->OnMouseHovered(position, snapped, dx, dy);
	}
}

void Navigator::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {
	if (selectedTool) {
		selectedTool->OnMouseDragged(position, snapped, dx, dy);
	}
}

void Navigator::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel) {
	if (selectedTool) {
		selectedTool->OnSpaceClicked(position, snapped, left, right, wheel);
	}
}

void Navigator::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {
	if (selectedTool) {
		selectedTool->OnShapeClicked(position, snapped, left, right, wheel, shape);
	}
}

void Navigator::OnToolChanged() {

	CancelShape();

	if (selectedTool) {
		selectedTool->OnToolChanged();
	}

}

void Navigator::OnLayerSelected(LayerID layer) {

	file.ActivateLayer(layer);

	if (selectedTool) {
		selectedTool->OnLayerSelected(layer);
	}
}

void Navigator::OnEscapePressed() {

	if (selectedTool) {
		if (!selectedTool->StepToolBack()) {	// If false, it couldn't be stepped back further
			UseTool(ToolType::SELECT);
		}
	}
	else {
		UseTool(ToolType::SELECT);
	}
}






void Navigator::SelectAll() {

	if (selectedTool) {
		selectedTool->SelectAll();
	}
}

void Navigator::UndoAction() {
	file.UndoAction();
}

void Navigator::CopyClipboard() {

	if (selectedTool) {
		selectedTool->CopyClipboard();
	}
}

void Navigator::CutClipboard() {

	if (selectedTool) {
		selectedTool->CutClipboard();
	}
}

void Navigator::PasteClipboard() {

	if (selectedTool) {
		selectedTool->PasteClipboard();
	}
}

bool Navigator::OpenFile() {
	return file.OpenFile();
}

bool Navigator::OpenEmptyFile() {
	return file.OpenEmptyFile();
}

bool Navigator::OpenFile(const std::string& path) {
	return file.OpenFile(path);
}

bool Navigator::SaveFile() {
	return file.SaveFile();
}

bool Navigator::SaveFileAs() {
	return file.SaveFile(true);
}

void Navigator::ResetGui() {

	// Simply delete the .ini file
	if (Battery::FileUtils::FileExists(imguiFileLocation)) {
		Battery::FileUtils::RemoveFile(imguiFileLocation);
	}
}

void Navigator::ResetViewport() {

	panOffset = { 0, 0 };
	scale = 7;

}

bool Navigator::ExportClipboardRendering() {
	GetClientApplication()->window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_BUSY);
	auto image = file.ExportImage(exportTransparent, exportDPI);

	if (!image.IsValid())
		return false;

	bool success = GetClientApplication()->window.SetClipboardImage(image);
	GetClientApplication()->window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
	return success;
}

bool Navigator::LoadSettings() {

	try {
		std::string path = GetSettingsDirectory() + SETTINGS_FILENAME;
		auto file = Battery::FileUtils::ReadFile(path);

		if (file.fail()) {
			LOG_ERROR("Failed to load settings file!");
			return false;
		}

		nlohmann::json json = nlohmann::json::parse(file.content());

		if (json["settings_type"] != JSON_SETTINGS_TYPE) {
			LOG_ERROR("Can't load settings file: Invalid settings type!");
			return false;
		}
		if (json["settings_version"] != JSON_SETTINGS_VERSION) {
			LOG_ERROR("Can't load settings file: Invalid settings type!");
			return false;
		}

		exportDPI = json["export_dpi"];
		exportTransparent = json["export_transparent"];
		keepUpToDate = json["keep_up_to_date"];

		return true;
	}
	catch (...) {
		LOG_WARN("Failed to load settings file!");
	}

	return false;
}

bool Navigator::SaveSettings() {

	try {
		nlohmann::json json = nlohmann::json();

		json["settings_type"] = JSON_SETTINGS_TYPE;
		json["settings_version"] = JSON_SETTINGS_VERSION;

		json["export_dpi"] = exportDPI;
		json["export_transparent"] = exportTransparent;
		json["keep_up_to_date"] = keepUpToDate;

		std::string file = GetSettingsDirectory() + SETTINGS_FILENAME;
		return Battery::FileUtils::WriteFile(file, json.dump(4));
	}
	catch (...) {
		LOG_WARN("Failed to save settings file!");
	}

	return false;
}








std::string Navigator::GetMostRecentFile() {
	auto files = GetRecentFiles();

	if (files.size() > 0) {
		return files[files.size() - 1];
	}

	return "";
}

std::vector<std::string> Navigator::GetRecentFiles() {
	auto file = Battery::FileUtils::ReadFile(GetSettingsDirectory() + RECENT_FILES_FILENAME);

	if (file.fail()) {
		LOG_ERROR("Can't read file with recent files: '{}'!", file.path());
		return std::vector<std::string>();
	}

	return Battery::StringUtils::SplitString(file.content(), '\n');
}

bool Navigator::AppendRecentFile(std::string recentFile) {
	auto files = GetRecentFiles();		// If the file can't be found and vector is empty, doesn't matter

	// If it's already there, delete and append again
	for (size_t i = 0; i < files.size(); i++) {
		if (files[i] == recentFile) {
			files.erase(files.begin() + i);
			break;
		}
	}
	files.push_back(recentFile);

	// Delete first one, if there's too many
	while (files.size() > MAX_NUMBER_OF_RECENT_FILES) {
		files.erase(files.begin());
	}

	return SaveRecentFiles(files);
}

bool Navigator::SaveRecentFiles(std::vector<std::string> recentFiles) {
	auto file = Battery::StringUtils::JoinStrings(recentFiles, "\n");
	return Battery::FileUtils::WriteFile(GetSettingsDirectory() + RECENT_FILES_FILENAME, file);
}

std::string Navigator::GetSettingsDirectory() {
	return Battery::FileUtils::GetAllegroStandardPath(ALLEGRO_USER_SETTINGS_PATH);
}

std::string Navigator::GetApplicationVersion() {
#ifdef BATTERY_DEBUG
	auto file = Battery::FileUtils::ReadFile(Battery::FileUtils::GetExecutableDirectory() + "../../version");
#else
	auto file = Battery::FileUtils::ReadFile(Battery::FileUtils::GetExecutableDirectory() + "../version");
#endif

	if (file.fail()) {
		return "";
	}

	return file.content();
}

void Navigator::OpenNewWindowFile(const std::string& file) {

	if (Battery::FileUtils::FileExists(file)) {
		LOG_INFO("Starting new application instance while opening file '{}'", file);

		// Execute the first command line argument, which is always the path of the exe
		system(std::string("start " + GetClientApplication()->args[0] + " " + file).c_str());
	}
	else {
		LOG_ERROR("File can not be found: '{}'", file);
		Battery::ShowInfoMessageBox("The file '" + file + "' can not be found!");
	}
}

void Navigator::StartNewApplicationInstance() {
	LOG_INFO("Starting new instance of the application");

	// Execute the first command line argument, which is always the path of the exe
	system(std::string("start " + GetClientApplication()->args[0] + " new").c_str());
}

void Navigator::CloseApplication() {

	// Skip if a popup is open

	if (popupDeleteLayerOpen || popupExportOpen || popupSettingsOpen) {
		LOG_WARN("Can't exit: A popup is still open!");
		return;
	}

	// Only close application, if file is saved

	if (!file.ContainsChanges()) {
		GetClientApplication()->CloseApplication();
		return;
	}

	bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
		"Do you want to save the file?", GetClientApplication()->window.allegroDisplayPointer);

	if (!save) {	// Discard changes and close the application
		GetClientApplication()->CloseApplication();
		return;
	}

	if (SaveFile()) {
		GetClientApplication()->CloseApplication();
		return;
	}
}






void Navigator::AddLayer() {
	file.PushLayer();
}

void Navigator::AddLine(const LineShape& line) {

	// Safety check
	if (line.GetPoint1() == line.GetPoint2()) {
		LOG_WARN(__FUNCTION__ "(): Line is not added to buffer: Start and end points are identical!");
		return;
	}

	file.AddShape(ShapeType::LINE, line.GetPoint1(), line.GetPoint2(), line.GetThickness(), line.GetColor());

	LOG_TRACE(__FUNCTION__"(): Line was added");
}

void Navigator::AddCircle(const CircleShape& circle) {

	// Safety check
	if (circle.GetRadius() == 0) {
		LOG_WARN(__FUNCTION__ "(): Circle is not added to buffer: Radius is 0!");
		return;
	}

	file.AddShape(ShapeType::CIRCLE, circle.GetCenter(), circle.GetRadius(), circle.GetThickness(), circle.GetColor());

	LOG_TRACE(__FUNCTION__"(): Circle was added");
}

void Navigator::AddArc(const ArcShape& arc) {

	// Safety check
	if (arc.GetRadius() == 0) {
		LOG_WARN(__FUNCTION__ "(): Arc is not added to buffer: Radius is 0!");
		return;
	}

	file.AddShape(ShapeType::ARC, arc.GetCenter(), arc.GetRadius(), arc.GetStartAngle(), arc.GetEndAngle(), arc.GetThickness(), arc.GetColor());

	LOG_TRACE(__FUNCTION__"(): Arc was added");
}

































void Navigator::RenderShapes() {
	using namespace Battery;
	
	// Render in reverse order, except for the active layer
	auto& layers = file.GetLayers();
	for (size_t layerIndex = layers.size() - 1; layerIndex < layers.size(); layerIndex--) {
		auto& layer = layers[layerIndex];

		if (file.GetActiveLayer().GetID() == layer.GetID()) {	// Layer is the active one, skip
			continue;
		}

		for (auto& shape : layer.GetShapes()) {

			// Skip the shape if it's not on the screen
			if (shape->ShouldBeRendered(GetClientApplication()->window.GetWidth(), 
										GetClientApplication()->window.GetHeight()))
			{
				// Render the shape
				ShapeID id = shape->GetID();
				bool shapeSelected = false;
				ShapeID shapeHovered = false;

				if (selectedTool) {
					if (selectedTool->GetType() == ToolType::SELECT) {
						shapeSelected = static_cast<SelectionTool*>(selectedTool)->selectionHandler.IsShapeSelected(id);
						shapeHovered = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetLastHoveredShape();
					}
				}

				shape->Render(false, shapeSelected, id == shapeHovered);
			}
			else {
				LOG_TRACE(__FUNCTION__ "(): Skipping rendering shape #{}: Not on screen", shape->GetID());
			}
		}
	}

	// Now render the active layer
	for (auto& shape : file.GetActiveLayer().GetShapes()) {

		// Skip the shape if it's not on the screen
		if (shape->ShouldBeRendered(GetClientApplication()->window.GetWidth(),
			GetClientApplication()->window.GetHeight()))
		{
			// Render the shape
			ShapeID id = shape->GetID();
			bool shapeSelected = false;
			ShapeID shapeHovered = false;

			if (selectedTool) {
				if (selectedTool->GetType() == ToolType::SELECT) {
					shapeSelected = static_cast<SelectionTool*>(selectedTool)->selectionHandler.IsShapeSelected(id);
					shapeHovered = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetLastHoveredShape();
				}
			}

			shape->Render(true, shapeSelected, id == shapeHovered);
		}
		else {
			LOG_TRACE(__FUNCTION__ "(): Skipping rendering shape #{}: Not on screen", shape->GetID());
		}
	}

	// Now render all selected shapes again, so the highlighted ones are on top
	for (const auto& shape : file.GetActiveLayer().GetShapes()) {

		// Skip the shape if it's not on the screen
		if (shape->ShouldBeRendered(GetClientApplication()->window.GetWidth(),
			GetClientApplication()->window.GetHeight()))
		{
			// Render the shape on top of all others
			ShapeID id = shape->GetID();
			bool shapeSelected = false;
			ShapeID shapeHovered = false;

			if (selectedTool) {
				if (selectedTool->GetType() == ToolType::SELECT) {
					shapeSelected = static_cast<SelectionTool*>(selectedTool)->selectionHandler.IsShapeSelected(id);
					shapeHovered = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetLastHoveredShape();

					if (shapeSelected || id == shapeHovered) {
						shape->Render(true, shapeSelected, id == shapeHovered);
					}
				}
			}
		}
	}
}
