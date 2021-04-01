
#include "pch.h"
#include "Navigator.h"
#include "Layer.h"
#include "config.h"
#include "Application.h"

#include "Tools/SelectionTool.h"
#include "Tools/LineTool.h"
#include "Tools/LineStripTool.h"

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
	
	// Handle all queued events
	UpdateEvents();

	// Print all shapes
	//PrintShapes();
}

void Navigator::OnRender() {

	ApplicationRenderer::BeginFrame();

	// Draw first part of the selection box
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool.get())->RenderFirstPart();
		}
	}
	
	// Main elements of the application
	ApplicationRenderer::DrawGrid();
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
			static_cast<SelectionTool*>(selectedTool.get())->RenderSecondPart();
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
		if (event.button & 0x01) {
			OnMouseLeftClicked(position, snapped);
		}
		else if (event.button & 0x02) {
			OnMouseRightClicked(position, snapped);
		}
		else if(event.button & 0x04) {
			OnMouseWheelClicked(position, snapped);
		}
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

		OnMouseReleased(position);
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

		OnMouseMoved(position, snapped);
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
		selectedTool.release();
		break;

	case ToolType::SELECT:
		selectedTool = std::make_unique<SelectionTool>();
		break;

	case ToolType::LINE:
		selectedTool = std::make_unique<LineTool>();
		break;

	case ToolType::LINE_STRIP:
		selectedTool = std::make_unique<LineStripTool>();
		break;

	default:
		LOG_WARN("Unsupported tool type was selected");
		selectedTool.release();
		break;
	}

	OnToolChanged();
}

void Navigator::PrintShapes() {
	
	// Print all shapes in the currently selected Layer
	LOG_WARN("Layer #{}: Name '{}'", file.GetActiveLayer().GetID(), file.GetActiveLayer().name);
	for (const auto& shape : file.GetActiveLayerShapes()) {
		LOG_WARN("Shape #{}: ", shape->GetID());
		LOG_ERROR("Shape JSON Content: \n{}", shape->GetJson().dump(4));
	}
}

void Navigator::RemoveSelectedShapes() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool.get())->RemoveSelectedShapes();
		}
	}
}

void Navigator::MoveSelectedShapesLeft() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool.get())->MoveSelectedShapesLeft(snapSize);
		}
	}
}

void Navigator::MoveSelectedShapesRight() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool.get())->MoveSelectedShapesRight(snapSize);
		}
	}
}

void Navigator::MoveSelectedShapesUp() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool.get())->MoveSelectedShapesUp(snapSize);
		}
	}
}

void Navigator::MoveSelectedShapesDown() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool.get())->MoveSelectedShapesDown(snapSize);
		}
	}
}

void Navigator::SelectNextPossibleShape() {
	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			static_cast<SelectionTool*>(selectedTool.get())->SelectNextPossibleShape();
		}
	}
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

	case ALLEGRO_KEY_Z:
		// Undo previous action
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			file.GetActiveLayer().UndoAction();
		}
		break;

	case ALLEGRO_KEY_A:		// Select all
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			SelectAll();
		}
		break;

	case ALLEGRO_KEY_P:		// Print
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			Print();
		}
		break;

	case ALLEGRO_KEY_O:		// Open
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {	// Get fresh key state
			OpenFile();
		}
		break;

	case ALLEGRO_KEY_S:		// Save
		if (GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL) &&
			GetClientApplication()->GetKey(ALLEGRO_KEY_LCTRL)) {
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
			PasteClipboard();
		}
		break;

	default:
		break;

	}
}

void Navigator::OnMouseLeftClicked(const glm::vec2& position, const glm::vec2& snapped) {

	ShapeID shapeClicked = -1;

	if (selectedTool) {
		if (selectedTool->GetType() == ToolType::SELECT) {
			shapeClicked = static_cast<SelectionTool*>(selectedTool.get())->selectionHandler.GetHoveredShape(position);
		}
	}

	if (shapeClicked == -1) {
		OnSpaceClicked(position, snapped);
	}
	else {
		OnShapeClicked(position, snapped, shapeClicked);
	}
}

void Navigator::OnMouseRightClicked(const glm::vec2& position, const glm::vec2& snapped) {

	CancelShape();
}

void Navigator::OnMouseWheelClicked(const glm::vec2& position, const glm::vec2& snapped) {

	LOG_WARN("MOUSE WHEEL CLICKED");

}

void Navigator::OnMouseReleased(const glm::vec2& position) {
	if (selectedTool) {
		selectedTool->OnMouseReleased(position);
	}
}

void Navigator::OnMouseMoved(const glm::vec2& position, const glm::vec2& snapped) {
	using namespace Battery;

	if (GetClientApplication()->window.GetLeftMouseButton()) {
		OnMouseDragged(position, snapped);
	}
	else {
		OnMouseHovered(position, snapped);
	}

}

void Navigator::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped) {
	if (selectedTool) {
		selectedTool->OnMouseHovered(position, snapped);
	}
}

void Navigator::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped) {
	if (selectedTool) {
		selectedTool->OnMouseDragged(position, snapped);
	}
}

void Navigator::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped) {
	if (selectedTool) {
		selectedTool->OnSpaceClicked(position, snapped);
	}
}

void Navigator::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, ShapeID shape) {
	if (selectedTool) {
		selectedTool->OnShapeClicked(position, snapped, shape);
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






void Navigator::SelectAll() {

	if (selectedTool) {
		selectedTool->SelectAll();
	}
}

void Navigator::Print() {
	LOG_ERROR("PRINT NOW");
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

void Navigator::OpenFile() {
	LOG_ERROR("OPEN FILE NOW");
}

void Navigator::SaveFile() {
	LOG_ERROR("SAVE FILE NOW");
}

void Navigator::SaveFileAs() {
	LOG_ERROR("SAVE FILE AS NOW");
}






void Navigator::AddLayer() {
	file.PushLayer();
}

void Navigator::AddLine(const glm::vec2& p1, const glm::vec2& p2) {

	// Safety check
	if (p1 == p2) {
		LOG_WARN(__FUNCTION__ "(): Line is not added to buffer: Start and end points are identical!");
		return;
	}

	file.GetActiveLayer().SaveState();
	file.GetActiveLayer().AddShape(ShapeType::LINE, p1, p2, currentLineThickness, currentShapeColor);
	file.FileChanged();

	LOG_TRACE(__FUNCTION__"(): Line was added");
}

































void Navigator::RenderShapes() {
	using namespace Battery;
	
	// Render in reverse order
	auto& layers = file.GetLayers();
	for (size_t layerIndex = layers.size() - 1; layerIndex < layers.size(); layerIndex--) {
		auto& layer = layers[layerIndex];

		for (const auto& shape : layer.GetShapes()) {

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
						shapeSelected = static_cast<SelectionTool*>(selectedTool.get())->selectionHandler.IsShapeSelected(id);
						shapeHovered = static_cast<SelectionTool*>(selectedTool.get())->selectionHandler.GetLastHoveredShape();
					}
				}

				shape->Render(layer.GetID() == file.GetActiveLayer().GetID(),
					shapeSelected, id == shapeHovered);
			}
			else {
				LOG_TRACE(__FUNCTION__ "(): Skipping rendering shape #{}: Not on screen", shape->GetID());
			}
		}
	}
}
