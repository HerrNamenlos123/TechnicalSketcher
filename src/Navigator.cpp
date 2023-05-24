
#include "pch.h"

#include "Navigator.h"
#include "config.h"

#include "Tools/SelectionTool.h"
#include "Tools/LineTool.h"
#include "Tools/LineStripTool.h"
#include "Tools/CircleTool.h"

void Navigator::CreateInstance() {
	instance.reset(new Navigator());
}

void Navigator::DestroyInstance() {
	instance.reset();
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
	//clipboardShapeFormat = Battery::GetApp().window.RegisterClipboardFormat(CLIPBOARD_FORMAT);
	b::log::error("CLIPBOARD");

	// Load the location of the ImGui .ini file
	imguiFileLocation = GetSettingsDirectory() + IMGUI_FILENAME;
}

void Navigator::OnDetach() {
}

void Navigator::OnUpdate() {
	//windowSize = glm::ivec2(Battery::GetApp().window.getSize().x, Battery::GetApp().window.getSize().y);
	mousePosition = ConvertScreenToWorkspaceCoords({ sf::Mouse::getPosition().x, sf::Mouse::getPosition().y });
	mouseSnapped = round(mousePosition / snapSize) * snapSize;

	// Key control
	controlKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
	shiftKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
	
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

	//Battery::Renderer2D::DrawBackground(file.canvasColor);

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
}

void Navigator::OnEvent(sf::Event e, bool& handled) {
	
	switch (e.type) {
	case sf::Event::MouseWheelScrolled:
	case sf::Event::MouseButtonPressed:
	case sf::Event::MouseButtonReleased:
	case sf::Event::MouseMoved:
	case sf::Event::KeyPressed:
	case sf::Event::KeyReleased:
		eventBuffer.push_back(e);
		handled = true;
		break;

	default:
		break;
	}
}






ImVec2 Navigator::ConvertScreenToWorkspaceCoords(const ImVec2& v) {
	return (v - panOffset - ImVec2(Battery::GetApp().window.getSize().x, 
		Battery::GetApp().window.getSize().y) * 0.5f) / scale;
}

ImVec2 Navigator::ConvertWorkspaceToScreenCoords(const ImVec2& v) {
	return panOffset + v * scale + ImVec2(Battery::GetApp().window.getSize().x,
		Battery::GetApp().window.getSize().y) * 0.5f;
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

	auto mPos = sf::Mouse::getPosition();
	ImVec2 mouseToCenter = ImVec2(panOffset.x - mPos.x + windowSize.x / 2.f,
										panOffset.y - mPos.y + windowSize.y / 2.f);

	if (scroll > 0)
		panOffset += mouseToCenter * factor - mouseToCenter;
	else
		panOffset -= mouseToCenter - mouseToCenter / factor;
}

void Navigator::UpdateEvents() {

	for (sf::Event event : eventBuffer) {

		ImVec2 position = ConvertScreenToWorkspaceCoords({ event.mouseButton.x, event.mouseButton.y });	// Allow smooth positioning when CTRL is pressed
		ImVec2 snapped = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ? round(position / snapSize) * snapSize : position;
		
		bool left = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		bool right = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
		bool wheel = sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);

		switch (event.type) {

		case sf::Event::MouseWheelScrolled:
			MouseScrolled(event.mouseWheelScroll.delta);
			break;

		case sf::Event::MouseButtonPressed:		
			OnMouseClicked(position, snapped, left, right, wheel);
			break;

		case sf::Event::MouseButtonReleased:	
			OnMouseReleased(position, left, right, wheel);
			break;

		case sf::Event::MouseMoved:
			OnMouseMoved(position, snapped, event.mouseMove.x, event.mouseMove.y);
			break;

		case sf::Event::KeyPressed:
			OnKeyPressed(event);
			break;

		case sf::Event::KeyReleased:
			OnKeyReleased(event);
			break;

		default:
			break;
		}

	}
	eventBuffer.clear();
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
		b::log::warn("Can't choose tool 'NONE'");
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
		b::log::warn("Unsupported tool type was selected");
		selectedTool = &selectionTool;
		break;
	}

	OnToolChanged();
}

void Navigator::PrintShapes() {
	
	// Print all shapes in the currently selected Layer
	b::log::warn("Layer #{}: Name '{}'", file.GetActiveLayer().GetID(), file.GetActiveLayer().name);
	for (const auto& shape : file.GetActiveLayer().GetShapes()) {
		b::log::warn("Shape #{}: ", shape->GetID());
		b::log::error("Shape JSON Content: \n{}", shape->GetJson().dump(4));
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








void Navigator::OnKeyPressed(sf::Event event) {
	
	switch (event.key.code) {

	case sf::Keyboard::Tab:
		// Switch through all possibly selected shapes, is wrapped around automatically
		SelectNextPossibleShape();
		break;

	case sf::Keyboard::LControl:
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) ||
			sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) ||
			sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
		{
			OnMouseDragged(mousePosition, mouseSnapped, 0, 0);
		}
		else {
			OnMouseHovered(mousePosition, mouseSnapped, 0, 0);
		}
		break;

	case sf::Keyboard::Delete:
		// Delete selected shapes
		RemoveSelectedShapes();
		break;

	case sf::Keyboard::Left:
		// Move shape to the left by one unit
		MoveSelectedShapesLeft();
		break;

	case sf::Keyboard::Right:
		// Move shape to the right by one unit
		MoveSelectedShapesRight();
		break;

	case sf::Keyboard::Up:
		// Move shape up by one unit
		MoveSelectedShapesUp();
		break;

	case sf::Keyboard::Down:
		// Move shape down by one unit
		MoveSelectedShapesDown();
		break;

	case sf::Keyboard::Escape:
		// Reset tools
		OnEscapePressed();
		break;

	case sf::Keyboard::Z:
		// Undo previous action
		if (event.key.control) {
			UndoAction();
		}
		break;

	case sf::Keyboard::A:		// Select all
		if (event.key.control) {
			UseTool(ToolType::SELECT);
			SelectAll();
		}
		break;

	case sf::Keyboard::O:		// Open
		if (event.key.control) {	// Get fresh key state
			OpenFile();
		}
		break;

	case sf::Keyboard::S:		// Save
		if (event.key.control) {
			if (event.key.shift) SaveFileAs();	// CTRL+SHIFT+S
			else SaveFile();					// CTRL+S
		}
		break;

	case sf::Keyboard::C:		// Copy
		if (event.key.control) {
			CopyClipboard();
		}
		break;

	case sf::Keyboard::X:		// Cut
		if (event.key.control) {
			CutClipboard();
		}
		break;

	case sf::Keyboard::V:		// Paste
		if (event.key.control) {
			UseTool(ToolType::SELECT);
			PasteClipboard();
		}
		break;

	case sf::Keyboard::N:
		if (event.key.control) {
			if (event.key.shift) {
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

void Navigator::OnKeyReleased(sf::Event event) {
	
	switch (event.key.code) {

	case sf::Keyboard::LControl:
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || 
			sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) ||
			sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
		{
			OnMouseDragged(mousePosition, mouseSnapped, 0, 0);
		}
		else {
			OnMouseHovered(mousePosition, mouseSnapped, 0, 0);
		}
		break;

	default:
		break;

	}
}

void Navigator::OnMouseClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel) {

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

void Navigator::OnMouseReleased(const ImVec2& position, bool left, bool right, bool wheel) {
	if (selectedTool) {
		selectedTool->OnMouseReleased(position, left, right, wheel);
	}
}

void Navigator::OnMouseMoved(const ImVec2& position, const ImVec2& snapped, float dx, float dy) {
	using namespace Battery;

	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) ||
		sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) ||
		sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
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

void Navigator::OnMouseHovered(const ImVec2& position, const ImVec2& snapped, float dx, float dy) {
	if (selectedTool) {
		selectedTool->OnMouseHovered(position, snapped, dx, dy);
	}
}

void Navigator::OnMouseDragged(const ImVec2& position, const ImVec2& snapped, float dx, float dy) {
	if (selectedTool) {
		selectedTool->OnMouseDragged(position, snapped, dx, dy);
	}
}

void Navigator::OnSpaceClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel) {
	if (selectedTool) {
		selectedTool->OnSpaceClicked(position, snapped, left, right, wheel);
	}
}

void Navigator::OnShapeClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {
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

bool Navigator::OpenFile(const std::string& path, bool silent) {
	return file.OpenFile(path, silent);
}

bool Navigator::SaveFile() {
	return file.SaveFile();
}

bool Navigator::SaveFileAs() {
	return file.SaveFile(true);
}

void Navigator::ResetGui() {

	// Simply delete the .ini file
	if (Battery::FileExists(imguiFileLocation)) {
		Battery::RemoveFile(imguiFileLocation);
	}
}

void Navigator::ResetViewport() {

	panOffset = { 0, 0 };
	scale = 7;

}

bool Navigator::ExportToClipboard() {
	Battery::SetMouseCursor(sf::Cursor::Type::Wait);
	std::optional<sf::Image> image = file.ExportImage(exportTransparent, exportDPI);
	if (image)
		return false;

	//bool success = Battery::SetClipboardImage(image.value());
	b::log::error("CLIPBOARD NOW");
	Battery::SetMouseCursor(sf::Cursor::Type::Arrow);
	//return success;
	return false;
}

bool Navigator::ExportToFile() {
	
	std::string filename = Battery::PromptFileSaveDialog({ "*.png" }, Battery::GetApp().window.getSystemHandle());
	if (filename == "")
		return false;

	if (Battery::GetExtension(filename) != ".png") {
		filename += ".png";
	}

	// TODO: Overwrite message if png is added
	std::optional<sf::Image> image = file.ExportImage(exportTransparent, exportDPI);
	if (!image)
		return false;

	/*bool success = image.SaveToFile(filename);
	Battery::ExecuteShellCommand("explorer.exe /select," + filename);	// TODO: Make for linux
	return success;*/
	b::log::error("EXPORT NOW");
	return false;
}

bool Navigator::LoadSettings() {

	try {
		std::string path = GetSettingsDirectory() + SETTINGS_FILENAME;
		auto file = Battery::ReadFile(path);

		if (file.fail()) {
			b::log::error("Failed to load settings file!");
			return false;
		}

		nlohmann::json json = nlohmann::json::parse(file.content());

		if (json["settings_type"] != JSON_SETTINGS_TYPE) {
			b::log::error("Can't load settings file: Invalid settings type!");
			return false;
		}
		if (json["settings_version"] != JSON_SETTINGS_VERSION) {
			b::log::error("Can't load settings file: Invalid settings type!");
			return false;
		}

		exportDPI = json["export_dpi"];
		exportTransparent = json["export_transparent"];
		keepUpToDate = json["keep_up_to_date"];

		return true;
	}
	catch (...) {
		b::log::warn("Failed to load settings file!");
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
		return Battery::WriteFile(file, json.dump(4));
	}
	catch (...) {
		b::log::warn("Failed to save settings file!");
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
	auto file = Battery::ReadFile(GetSettingsDirectory() + RECENT_FILES_FILENAME);

	if (file.fail()) {
		b::log::error("Can't read file with recent files: '{}'!", file.path());
		return std::vector<std::string>();
	}

	return Battery::SplitString(file.content(), '\n');
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
	auto file = Battery::JoinStrings(recentFiles, "\n");
	return Battery::WriteFile(GetSettingsDirectory() + RECENT_FILES_FILENAME, file);
}

std::string Navigator::GetSettingsDirectory() {
	return "";
	//return Battery::GetAllegroStandardPath(ALLEGRO_USER_SETTINGS_PATH);
}

std::string Navigator::GetApplicationVersion() {
	return APP_VERSION;
}

void Navigator::OpenNewWindowFile(const std::string& file) {

	if (Battery::FileExists(file)) {
		LOG_INFO("Starting new application instance while opening file '{}'", file);

		// Execute the first command line argument, which is always the path of the exe
		system(std::string("start " + Battery::GetApp().args[0] + " " + file).c_str());
	}
	else {
		b::log::error("File can not be found: '{}'", file);
		//Battery::ShowInfoMessageBox("The file '" + file + "' can not be found!");
	}
}

void Navigator::StartNewApplicationInstance() {
	LOG_INFO("Starting new instance of the application");

	// Execute the first command line argument, which is always the path of the exe
	system(std::string("start " + Battery::GetApp().args[0] + " new").c_str());
}

bool Navigator::CloseApplication() {

	// Skip if a popup is open

	if (popupDeleteLayerOpen || popupExportOpen || popupSettingsOpen) {
		b::log::warn("Can't exit: A popup is still open!");
		return false;
	}

	// Only close application, if file is saved

	if (!file.ContainsChanges()) {
		Battery::GetApp().CloseApplication();
		return true;
	}

	//bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
	//	"Do you want to save the file?", Battery::GetApp().window.getSystemHandle());
	bool save = false;

	if (!save) {	// Discard changes and close the application
		Battery::GetApp().CloseApplication();
		return true;
	}

	if (SaveFile()) {
		Battery::GetApp().CloseApplication();
		return true;
	}

	return false;
}






void Navigator::AddLayer() {
	file.PushLayer();
}

void Navigator::AddLine(const LineShape& line) {

	// Safety check
	if (line.GetPoint1() == line.GetPoint2()) {
		b::log::warn(__FUNCTION__ "(): Line is not added to buffer: Start and end points are identical!");
		return;
	}

	file.AddShape(ShapeType::LINE, line.GetPoint1(), line.GetPoint2(), line.GetThickness(), line.GetColor());

	LOG_TRACE(__FUNCTION__"(): Line was added");
}

void Navigator::AddCircle(const CircleShape& circle) {

	// Safety check
	if (circle.GetRadius() == 0) {
		b::log::warn(__FUNCTION__ "(): Circle is not added to buffer: Radius is 0!");
		return;
	}

	file.AddShape(ShapeType::CIRCLE, circle.GetCenter(), circle.GetRadius(), circle.GetThickness(), circle.GetColor());

	LOG_TRACE(__FUNCTION__"(): Circle was added");
}

void Navigator::AddArc(const ArcShape& arc) {

	// Safety check
	if (arc.GetRadius() == 0) {
		b::log::warn(__FUNCTION__ "(): Arc is not added to buffer: Radius is 0!");
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
			if (shape->ShouldBeRendered(Battery::GetApp().window.getSize().x, 
										Battery::GetApp().window.getSize().y))
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
		if (shape->ShouldBeRendered(Battery::GetApp().window.getSize().x,
			Battery::GetApp().window.getSize().y))
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
		if (shape->ShouldBeRendered(Battery::GetApp().window.getSize().x,
			Battery::GetApp().window.getSize().y))
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
