
#include "Tsk.hpp"
#include "TskHost.hpp"
#include "TskSettings.hpp"

TskHost::TskHost() {
    addDocument();
    addDocument();
    addDocument();
    addDocument();
    addDocument();
}

void TskHost::addDocument() {
    m_documents.emplace_back();
    m_activeDocumentIndex = m_documents.size() - 1;
}

void TskHost::update() {
}

void TskHost::render() {
    m_panel.left = 0;
    m_panel.top = 0;
    m_panel.width = Tsk::get().mainWindow.getSize().x;
    m_panel.height = Tsk::get().mainWindow.getSize().y;
    m_panel.style["ImGuiStyleVar_WindowRounding"] = 0;
    m_tabs.left = 0;
    m_tabs.top = 0;
    m_tabs.width = "100%";
    m_tabs.height = "100%";

    m_panel([this]{
        m_tabs.items.resize(m_documents.size());
        for (size_t i = 0; i < m_documents.size(); i++) {
            m_tabs.items[i].label = m_documents[i].getVisualFilename();
            m_tabs.items[i].unsavedDocument = m_documents[i].containsUnsavedChanges();
        }
        m_tabs([this] {
            ImGui::Text("Hello, world %d", m_tabs.selectedItemArrayIndex.has_value() ? m_tabs.selectedItemArrayIndex.value() : -1);
        });
        for (size_t i = 0; i < m_documents.size(); i++) {
            if (!m_tabs.items[i].open) {
                m_documents.erase(m_documents.begin() + i);
                i--;
            }
        }



        ImGui::ShowDemoWindow();
    });
}


//
//#include "resources/tsk_version.hpp"
//
//#include "Tools/SelectionTool.h"
//#include "Tools/LineTool.h"
//#include "Tools/LineStripTool.h"
//#include "Tools/CircleTool.h"
//
//void Navigator::OnAttach() {
//	UseTool(ToolType::SELECT);
//
//	// Get the version of the application
//	m_applicationVersion = GetApplicationVersion();
//
//	// Register the clipboard format
//	//clipboardShapeFormat = Battery::GetApp().window.RegisterClipboardFormat(CLIPBOARD_FORMAT);
//	b::log::error("CLIPBOARD");
//
//	// Load the location of the ImGui .ini file
//    m_imguiFileLocation = TskSettings::SettingsDirectory() + IMGUI_FILENAME;
//}
//
//void Navigator::OnDetach() {
//}
//
//void Navigator::OnUpdate() {
//	//windowSize = glm::ivec2(Battery::GetApp().window.getSize().x, Battery::GetApp().window.getSize().y);
//	mousePosition = ConvertScreenToWorkspaceCoords({ (float)sf::Mouse::getPosition().x, (float)sf::Mouse::getPosition().y });
//	mouseSnapped = b::round(mousePosition / snapSize) * snapSize;
//
//	// Key control
//	controlKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
//	shiftKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
//
//	// Allow smooth positioning when CTRL is pressed
//	if (controlKeyPressed) {
//		mouseSnapped = mousePosition;
//	}
//
//	// Update window title
//	m_file.UpdateWindowTitle();
//
//	// Handle all queued events
//	UpdateEvents();
//
//	// Print all shapes
//	//PrintShapes();
//}
//
//void Navigator::OnRender() {
//
//	//Battery::Renderer2D::DrawBackground(file.canvasColor);
//
//	// Draw first part of the selection box
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			static_cast<SelectionTool*>(selectedTool)->RenderFirstPart();
//		}
//	}
//
//	// Main elements of the application
//	if (gridShown) {
//		ApplicationRenderer::DrawGrid(infiniteSheet);
//	}
//	RenderShapes();
//
//	// Draw shape preview
//	if (selectedTool) {
//		selectedTool->RenderPreview();
//	}
//
//	// Preview point
//	if (previewPointShown) {
//		ApplicationRenderer::DrawPreviewPoint(previewPointPosition);
//	}
//
//	// Draw second part of the selection box
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			static_cast<SelectionTool*>(selectedTool)->RenderSecondPart();
//		}
//	}
//}
//
//void Navigator::OnEvent(sf::Event e, bool& handled) {
//
//	switch (e.type) {
//	case sf::Event::MouseWheelScrolled:
//	case sf::Event::MouseButtonPressed:
//	case sf::Event::MouseButtonReleased:
//	case sf::Event::MouseMoved:
//	case sf::Event::KeyPressed:
//	case sf::Event::KeyReleased:
//		eventBuffer.push_back(e);
//		handled = true;
//		break;
//
//	default:
//		break;
//	}
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//void Navigator::UpdateEvents() {
//
//	for (sf::Event event : eventBuffer) {
//
//		ImVec2 position = ConvertScreenToWorkspaceCoords({ (float)event.mouseButton.x, (float)event.mouseButton.y });	// Allow smooth positioning when CTRL is pressed
//		ImVec2 snapped = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ? b::round(position / snapSize) * snapSize : position;
//
//		bool left = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
//		bool right = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
//		bool wheel = sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
//
//		switch (event.type) {
//
//		case sf::Event::MouseWheelScrolled:
//			MouseScrolled(event.mouseWheelScroll.delta);
//			break;
//
//		case sf::Event::MouseButtonPressed:
//			OnMouseClicked(position, snapped, left, right, wheel);
//			break;
//
//		case sf::Event::MouseButtonReleased:
//			OnMouseReleased(position, left, right, wheel);
//			break;
//
//		case sf::Event::MouseMoved:
//			OnMouseMoved(position, snapped, event.mouseMove.x, event.mouseMove.y);
//			break;
//
//		case sf::Event::KeyPressed:
//			OnKeyPressed(event);
//			break;
//
//		case sf::Event::KeyReleased:
//			OnKeyReleased(event);
//			break;
//
//		default:
//			break;
//		}
//
//	}
//	eventBuffer.clear();
//}
//
//void Navigator::CancelShape() {
//
//	if (selectedTool) {
//		selectedTool->CancelShape();
//	}
//	b::log::trace("Shape cancelled");
//
//}
//
//void Navigator::UseTool(enum class ToolType tool) {
//
//	switch (tool) {
//
//	case ToolType::NONE:
//		b::log::warn("Can't choose tool 'NONE'");
//		selectedTool = &selectionTool;
//		break;
//
//	case ToolType::SELECT:
//		selectedTool = &selectionTool;
//		break;
//
//	case ToolType::LINE:
//		selectedTool = &lineTool;
//		break;
//
//	case ToolType::LINE_STRIP:
//		selectedTool = &lineStripTool;
//		break;
//
//	case ToolType::CIRCLE:
//		selectedTool = &circleTool;
//		break;
//
//	case ToolType::ARC:
//		selectedTool = &arcTool;
//		break;
//
//	default:
//		b::log::warn("Unsupported tool type was selected");
//		selectedTool = &selectionTool;
//		break;
//	}
//
//	OnToolChanged();
//}
//
//void Navigator::PrintShapes() {
//
//	// Print all shapes in the currently selected Layer
//	b::log::warn("Layer #{}: Name '{}'", m_file.GetActiveLayer().GetID(), m_file.GetActiveLayer().name);
//	for (const auto& shape : m_file.GetActiveLayer().GetShapes()) {
//		b::log::warn("Shape #{}: ", shape->GetID());
//		b::log::error("Shape JSON Content: \n{}", shape->GetJson().dump(4));
//	}
//}
//
//void Navigator::RemoveSelectedShapes() {
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			static_cast<SelectionTool*>(selectedTool)->RemoveSelectedShapes();
//		}
//	}
//}
//
//void Navigator::MoveSelectedShapesLeft() {
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			static_cast<SelectionTool*>(selectedTool)->MoveSelectedShapesLeft(snapSize);
//		}
//	}
//}
//
//void Navigator::MoveSelectedShapesRight() {
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			static_cast<SelectionTool*>(selectedTool)->MoveSelectedShapesRight(snapSize);
//		}
//	}
//}
//
//void Navigator::MoveSelectedShapesUp() {
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			static_cast<SelectionTool*>(selectedTool)->MoveSelectedShapesUp(snapSize);
//		}
//	}
//}
//
//void Navigator::MoveSelectedShapesDown() {
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			static_cast<SelectionTool*>(selectedTool)->MoveSelectedShapesDown(snapSize);
//		}
//	}
//}
//
//void Navigator::SelectNextPossibleShape() {
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			static_cast<SelectionTool*>(selectedTool)->SelectNextPossibleShape();
//		}
//	}
//	tabbedShapeInfo = true;
//}
//
//
//
//
//
//
//
//
//void Navigator::OnKeyPressed(sf::Event event) {
//
//	switch (event.key.code) {
//
//	case sf::Keyboard::Tab:
//		// Switch through all possibly selected shapes, is wrapped around automatically
//		SelectNextPossibleShape();
//		break;
//
//	case sf::Keyboard::LControl:
//		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) ||
//			sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) ||
//			sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
//		{
//			OnMouseDragged(mousePosition, mouseSnapped, 0, 0);
//		}
//		else {
//			OnMouseHovered(mousePosition, mouseSnapped, 0, 0);
//		}
//		break;
//
//	case sf::Keyboard::Delete:
//		// Delete selected shapes
//		RemoveSelectedShapes();
//		break;
//
//	case sf::Keyboard::Left:
//		// Move shape to the left by one unit
//		MoveSelectedShapesLeft();
//		break;
//
//	case sf::Keyboard::Right:
//		// Move shape to the right by one unit
//		MoveSelectedShapesRight();
//		break;
//
//	case sf::Keyboard::Up:
//		// Move shape up by one unit
//		MoveSelectedShapesUp();
//		break;
//
//	case sf::Keyboard::Down:
//		// Move shape down by one unit
//		MoveSelectedShapesDown();
//		break;
//
//	case sf::Keyboard::Escape:
//		// Reset tools
//		OnEscapePressed();
//		break;
//
//	case sf::Keyboard::Z:
//		// Undo previous action
//		if (event.key.control) {
//			UndoAction();
//		}
//		break;
//
//	case sf::Keyboard::A:		// Select all
//		if (event.key.control) {
//			UseTool(ToolType::SELECT);
//			SelectAll();
//		}
//		break;
//
//	case sf::Keyboard::O:		// Open
//		if (event.key.control) {	// Get fresh key state
//			OpenFile();
//		}
//		break;
//
//	case sf::Keyboard::S:		// Save
//		if (event.key.control) {
//			if (event.key.shift) SaveFileAs();	// CTRL+SHIFT+S
//			else SaveFile();					// CTRL+S
//		}
//		break;
//
//	case sf::Keyboard::C:		// Copy
//		if (event.key.control) {
//			CopyClipboard();
//		}
//		break;
//
//	case sf::Keyboard::X:		// Cut
//		if (event.key.control) {
//			CutClipboard();
//		}
//		break;
//
//	case sf::Keyboard::V:		// Paste
//		if (event.key.control) {
//			UseTool(ToolType::SELECT);
//			PasteClipboard();
//		}
//		break;
//
//	case sf::Keyboard::N:
//		if (event.key.control) {
//			if (event.key.shift) {
//				Navigator::GetInstance()->StartNewApplicationInstance();	// CTRL + SHIFT + N
//			}
//			else {
//				Navigator::GetInstance()->OpenEmptyFile();		// CTRL + N
//				UseTool(ToolType::SELECT);
//			}
//		}
//		break;
//
//	default:
//		break;
//
//	}
//}
//
//void Navigator::OnKeyReleased(sf::Event event) {
//
//	switch (event.key.code) {
//
//	case sf::Keyboard::LControl:
//		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) ||
//			sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) ||
//			sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
//		{
//			OnMouseDragged(mousePosition, mouseSnapped, 0, 0);
//		}
//		else {
//			OnMouseHovered(mousePosition, mouseSnapped, 0, 0);
//		}
//		break;
//
//	default:
//		break;
//
//	}
//}
//
//void Navigator::OnMouseClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel) {
//
//	ShapeID shapeClicked = -1;
//
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			shapeClicked = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetHoveredShape(position);
//		}
//	}
//
//	if (shapeClicked == -1) {
//		OnSpaceClicked(position, snapped, left, right, wheel);
//	}
//	else {
//		OnShapeClicked(position, snapped, left, right, wheel, shapeClicked);
//	}
//}
//
//void Navigator::OnMouseReleased(const ImVec2& position, bool left, bool right, bool wheel) {
//	if (selectedTool) {
//		selectedTool->OnMouseReleased(position, left, right, wheel);
//	}
//}
//
//void Navigator::OnMouseMoved(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
//
//	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) ||
//		sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) ||
//		sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
//	{
//		OnMouseDragged(position, snapped, dx, dy);
//	}
//	else {
//		OnMouseHovered(position, snapped, dx, dy);
//	}
//
//	if (selectedTool) {
//		if (selectedTool->GetType() == ToolType::SELECT) {
//			ShapeID hovered = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetLastHoveredShape();
//			if (hovered == -1) {
//				tabbedShapeInfo = false;
//			}
//		}
//	}
//}
//
//void Navigator::OnMouseHovered(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
//	if (selectedTool) {
//		selectedTool->OnMouseHovered(position, snapped, dx, dy);
//	}
//}
//
//void Navigator::OnMouseDragged(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
//	if (selectedTool) {
//		selectedTool->OnMouseDragged(position, snapped, dx, dy);
//	}
//}
//
//void Navigator::OnSpaceClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel) {
//	if (selectedTool) {
//		selectedTool->OnSpaceClicked(position, snapped, left, right, wheel);
//	}
//}
//
//void Navigator::OnShapeClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {
//	if (selectedTool) {
//		selectedTool->OnShapeClicked(position, snapped, left, right, wheel, shape);
//	}
//}
//
//void Navigator::OnToolChanged() {
//
//	CancelShape();
//
//	if (selectedTool) {
//		selectedTool->OnToolChanged();
//	}
//
//}
//
//void Navigator::OnLayerSelected(LayerID layer) {
//
//    m_file.ActivateLayer(layer);
//
//	if (selectedTool) {
//		selectedTool->OnLayerSelected(layer);
//	}
//}
//
//void Navigator::OnEscapePressed() {
//
//	if (selectedTool) {
//		if (!selectedTool->StepToolBack()) {	// If false, it couldn't be stepped back further
//			UseTool(ToolType::SELECT);
//		}
//	}
//	else {
//		UseTool(ToolType::SELECT);
//	}
//}
//
//
//
//
//
//
//void Navigator::SelectAll() {
//
//	if (selectedTool) {
//		selectedTool->SelectAll();
//	}
//}
//
//void Navigator::UndoAction() {
//    m_file.UndoAction();
//}
//
//void Navigator::CopyClipboard() {
//
//	if (selectedTool) {
//		selectedTool->CopyClipboard();
//	}
//}
//
//void Navigator::CutClipboard() {
//
//	if (selectedTool) {
//		selectedTool->CutClipboard();
//	}
//}
//
//void Navigator::PasteClipboard() {
//
//	if (selectedTool) {
//		selectedTool->PasteClipboard();
//	}
//}
//
//bool Navigator::OpenFile() {
//	return m_file.OpenFile();
//}
//
//bool Navigator::OpenEmptyFile() {
//	return m_file.OpenEmptyFile();
//}
//
//bool Navigator::OpenFile(const b::fs::path& path, bool silent) {
//	return m_file.OpenFile(path, silent);
//}
//
//bool Navigator::SaveFile() {
//	return m_file.SaveFile();
//}
//
//bool Navigator::SaveFileAs() {
//	return m_file.SaveFile(true);
//}
//
//void Navigator::ResetGui() {
//
//	// Simply delete the .ini file
//	if (b::fs::exists(m_imguiFileLocation)) {
//		b::fs::remove(m_imguiFileLocation);
//	}
//}
//
//void Navigator::ResetViewport() {
//
//    m_panOffset = {0, 0 };
//	scale = 7;
//
//}
//
//bool Navigator::ExportToClipboard() {
////	b::windowed_application::get()->get_window(0)->getWindow().setMouseCursor(sf::Cursor::Type::Wait);
//	std::optional<sf::Image> image = m_file.ExportImage(exportTransparent, exportDPI);
//	if (image)
//		return false;
//
//	//bool success = Battery::SetClipboardImage(image.value());
//	b::log::error("CLIPBOARD NOW");
////    b::windowed_application::get()->get_window(0)->getWindow().setMouseCursor(sf::Cursor::Type::Arrow);
//	//return success;
//	return false;
//}
//
//bool Navigator::ExportToFile() {
//
//    b::filedialog dialog;
//    dialog.default_extension = "png";
//    dialog.filters = {{ "Image files", "*.png" }};
//
//	auto filename = dialog.sync_save();
//	if (filename.empty()) return false;
//
//	std::optional<sf::Image> image = m_file.ExportImage(exportTransparent, exportDPI);
//	if (!image)
//		return false;
//
//	bool success = image->saveToFile(filename);
//	b::execute(b::format("explorer.exe /select,{}", filename.string()));	// TODO: Make for linux
//	return success;
//}
//
//
//
//
//
//
//
//semver::version Navigator::GetApplicationVersion() {
//	return semver::from_string(resources::tsk_version.string());
//}
//
//void Navigator::OpenNewWindowFile(const b::fs::path& filepath) {
//
//	if (b::fs::exists(filepath)) {
//		b::log::info("Starting new application instance while opening file '{}'", filepath);
//
//		// Execute the first command line argument, which is always the path of the exe
//		b::execute(b::format("start {} {}", App::get().m_args[0], filepath));     // TODO: Make for linux
//	}
//	else {
//		b::log::error("File can not be found: '{}'", filepath);
//		//Battery::ShowInfoMessageBox("The file '" + file + "' can not be found!");
//	}
//}
//
//void Navigator::StartNewApplicationInstance() {
//	b::log::info("Starting new instance of the application");
//
//	// Execute the first command line argument, which is always the path of the exe
//    b::execute(b::format("start {} new", App::get().m_args[0]));     // TODO: Make for linux
//}
//
//bool Navigator::CloseApplication() {
//
//	// Skip if a popup is open
//
//	if (popupDeleteLayerOpen || popupExportOpen || popupSettingsOpen) {
//		b::log::warn("Can't exit: A popup is still open!");
//		return false;
//	}
//
//	// Only close application, if file is saved
//
//	if (!m_file.ContainsChanges()) {
//		App::get().stop_application();
//		return true;
//	}
//
//	//bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
//	//	"Do you want to save the file?", Battery::GetApp().window.getSystemHandle());
//	bool save = false;
//
//	if (!save) {	// Discard changes and close the application
//        App::get().stop_application();
//		return true;
//	}
//
//	if (SaveFile()) {
//        App::get().stop_application();
//		return true;
//	}
//
//	return false;
//}
//
//
//
//
//
//
//void Navigator::AddLayer() {
//	m_file.PushLayer();
//}
//
//void Navigator::AddLine(const LineShape& line) {
//
//	// Safety check
//	if (line.GetPoint1() == line.GetPoint2()) {
//		b::log::warn("{}(): Line is not added to buffer: Start and end points are identical!", std::source_location::current().function_name());
//		return;
//	}
//
//	m_file.AddShape(ShapeType::LINE, line.GetPoint1(), line.GetPoint2(), line.GetThickness(), line.GetColor());
//
//	b::log::trace("{}(): Line was added", std::source_location::current().function_name());
//}
//
//void Navigator::AddCircle(const CircleShape& circle) {
//
//	// Safety check
//	if (circle.GetRadius() == 0) {
//		b::log::warn("{}(): Circle is not added to buffer: Radius is 0!", std::source_location::current().function_name());
//		return;
//	}
//
//    m_file.AddShape(ShapeType::CIRCLE, circle.GetCenter(), circle.GetRadius(), circle.GetThickness(), circle.GetColor());
//
//	b::log::trace("{}(): Circle was added", std::source_location::current().function_name());
//}
//
//void Navigator::AddArc(const ArcShape& arc) {
//
//	// Safety check
//	if (arc.GetRadius() == 0) {
//		b::log::warn("{}(): Arc is not added to buffer: Radius is 0!", std::source_location::current().function_name());
//		return;
//	}
//
//	m_file.AddShape(ShapeType::ARC, arc.GetCenter(), arc.GetRadius(), arc.GetStartAngle(), arc.GetEndAngle(), arc.GetThickness(), arc.GetColor());
//
//	b::log::trace("{}(): Arc was added", std::source_location::current().function_name());
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//void Navigator::RenderShapes() {
//
//	// Render in reverse order, except for the active layer
//	auto& layers = m_file.GetLayers();
//	for (size_t layerIndex = layers.size() - 1; layerIndex < layers.size(); layerIndex--) {
//		auto& layer = layers[layerIndex];
//
//		if (m_file.GetActiveLayer().GetID() == layer.GetID()) {	// Layer is the active one, skip
//			continue;
//		}
//
//		for (auto& shape : layer.GetShapes()) {
//
//			// Skip the shape if it's not on the screen
//			if (shape->ShouldBeRendered(App::s_mainWindow->getSize().x,
//                                        App::s_mainWindow->getSize().y))
//			{
//				// Render the shape
//				ShapeID id = shape->GetID();
//				bool shapeSelected = false;
//				ShapeID shapeHovered = false;
//
//				if (selectedTool) {
//					if (selectedTool->GetType() == ToolType::SELECT) {
//						shapeSelected = static_cast<SelectionTool*>(selectedTool)->selectionHandler.IsShapeSelected(id);
//						shapeHovered = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetLastHoveredShape();
//					}
//				}
//
//				shape->Render(false, shapeSelected, id == shapeHovered);
//			}
//			else {
//				b::log::trace("{}(): Skipping rendering shape #{}: Not on screen", std::source_location::current().function_name(), shape->GetID());
//			}
//		}
//	}
//
//	// Now render the active layer
//	for (auto& shape : m_file.GetActiveLayer().GetShapes()) {
//
//		// Skip the shape if it's not on the screen
//		if (shape->ShouldBeRendered(App::s_mainWindow->getSize().x,
//                                    App::s_mainWindow->getSize().y))
//		{
//			// Render the shape
//			ShapeID id = shape->GetID();
//			bool shapeSelected = false;
//			ShapeID shapeHovered = false;
//
//			if (selectedTool) {
//				if (selectedTool->GetType() == ToolType::SELECT) {
//					shapeSelected = static_cast<SelectionTool*>(selectedTool)->selectionHandler.IsShapeSelected(id);
//					shapeHovered = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetLastHoveredShape();
//				}
//			}
//
//			shape->Render(true, shapeSelected, id == shapeHovered);
//		}
//		else {
//			b::log::trace("{}(): Skipping rendering shape #{}: Not on screen", std::source_location::current().function_name(), shape->GetID());
//		}
//	}
//
//	// Now render all selected shapes again, so the highlighted ones are on top
//	for (const auto& shape : m_file.GetActiveLayer().GetShapes()) {
//
//		// Skip the shape if it's not on the screen
//		if (shape->ShouldBeRendered(App::s_mainWindow->getSize().x, App::s_mainWindow->getSize().y)) {
//			// Render the shape on top of all others
//			ShapeID id = shape->GetID();
//			bool shapeSelected = false;
//			ShapeID shapeHovered = false;
//
//			if (selectedTool) {
//				if (selectedTool->GetType() == ToolType::SELECT) {
//					shapeSelected = static_cast<SelectionTool*>(selectedTool)->selectionHandler.IsShapeSelected(id);
//					shapeHovered = static_cast<SelectionTool*>(selectedTool)->selectionHandler.GetLastHoveredShape();
//
//					if (shapeSelected || id == shapeHovered) {
//						shape->Render(true, shapeSelected, id == shapeHovered);
//					}
//				}
//			}
//		}
//	}
//}
