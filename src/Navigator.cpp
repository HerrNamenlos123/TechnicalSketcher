
#include "pch.h"
#include "Navigator.h"
#include "Layer.h"
#include "config.h"
#include "Application.h"


Navigator::Navigator() {

}

void Navigator::OnAttach() {
	renderScene = new Battery::Scene(&applicationPointer->window);

	UseTool(CursorTool::SELECT);
}

void Navigator::OnDetach() {
	delete renderScene;
}

void Navigator::OnUpdate() {
	windowSize = glm::ivec2(applicationPointer->window.GetWidth(), applicationPointer->window.GetHeight());
	mousePosition = ConvertScreenToWorkspaceCoords(applicationPointer->window.GetMousePosition());
	mouseSnapped = round(mousePosition / snapSize) * snapSize;

	// Key control
	controlKeyPressed = applicationPointer->GetKey(ALLEGRO_KEY_LCTRL) || applicationPointer->GetKey(ALLEGRO_KEY_RCTRL);
	shiftKeyPressed = applicationPointer->GetKey(ALLEGRO_KEY_LSHIFT) || applicationPointer->GetKey(ALLEGRO_KEY_RSHIFT);
	
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
	using namespace Battery;

	Renderer2D::BeginScene(renderScene);
	Renderer2D::DrawBackground(BACKGROUND_COLOR);

	// Draw first part of the selection box
	if (selectionBoxActive) {			// Fill should be in the back
		glm::vec2 p1 = ConvertWorkspaceToScreenCoords(selectionBoxPointA);
		glm::vec2 p2 = ConvertWorkspaceToScreenCoords(selectionBoxPointB);
		Renderer2D::DrawRectangle(p1, p2, 2, { 0, 0, 0, 0 }, selectionBoxFillColor, 0);
	}
	
	// Main elements of the application
	DrawGrid();
	RenderShapes();
	
	// Various preview features
	switch (previewShape) {
	case ShapeType::LINE:
		DrawLine(previewShapePoint1, previewShapePoint2, currentLineThickness, { 0, 0, 0, 255 });
		break;
	}
	
	// Preview point
	if (previewPointShown) {
		float previewSize = 4;
		glm::vec2 pos = ConvertWorkspaceToScreenCoords(previewPointPosition);
		Renderer2D::DrawRectangle(pos - glm::vec2(previewSize / 2, previewSize / 2),
								  pos + glm::vec2(previewSize / 2, previewSize / 2),
								  1, { 0, 0, 0, 255 }, { 255, 255, 255, 255 }, 0);
	}

	//// Draw second part of the selection box
	if (selectionBoxActive) {			// Outline in the front
		glm::vec2 p1 = ConvertWorkspaceToScreenCoords(selectionBoxPointA);
		glm::vec2 p2 = ConvertWorkspaceToScreenCoords(selectionBoxPointB);
		Renderer2D::DrawRectangle(p1, p2, selectionBoxOutlineThickness, selectionBoxOutlineColor, { 0, 0, 0, 0 }, 0);
	}

	Renderer2D::EndScene();
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
	return (v - panOffset - glm::vec2(applicationPointer->window.GetWidth(), 
		applicationPointer->window.GetHeight()) * 0.5f) / scale;
}

glm::vec2 Navigator::ConvertWorkspaceToScreenCoords(const glm::vec2& v) {
	return panOffset + v * scale + glm::vec2(applicationPointer->window.GetWidth(), 
		applicationPointer->window.GetHeight()) * 0.5f;
}












void Navigator::SelectShape(ShapeID id) {
	if (!IsShapeSelected(id)) {
		selectedShapes.push_back(id);
	}
}

void Navigator::UnselectShape(ShapeID id) {

	for (size_t i = 0; i < selectedShapes.size(); i++) {
		if (selectedShapes[i] == id) {
			selectedShapes.erase(selectedShapes.begin() + i);
			return;
		}
	}
}

bool Navigator::IsShapeSelected(ShapeID id) {

	for (size_t i = 0; i < selectedShapes.size(); i++) {
		if (selectedShapes[i] == id) {
			return true;
		}
	}

	return false;
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

	auto mPos = applicationPointer->window.GetMousePosition();
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

	previewShape = ShapeType::INVALID;
	previewPointShown = true;
	previewPointPosition = mouseSnapped;
	LOG_TRACE("Shape cancelled");

}

void Navigator::UseTool(enum class CursorTool tool) {
	selectedTool = tool;
	OnToolChanged();
}

void Navigator::UpdateHoveredShapes() {

	// Update the highlighted shape
	possiblyHoveredShapes.clear();
	if (selectedTool == CursorTool::SELECT) {
		for (Shape& shape : file.GetActiveLayer()->GetShapes()) {
			if (shape.GetDistanceToCursor(mousePosition) * scale <= mouseHighlightThresholdDistance) {
				possiblyHoveredShapes.push_back(shape.shapeID);
			}
		}
		if (shapePossiblyChosen < possiblyHoveredShapes.size()) {
			lastHoveredShape = possiblyHoveredShapes[shapePossiblyChosen];
		}
		else if (possiblyHoveredShapes.size() >= 1) {
			lastHoveredShape = possiblyHoveredShapes[0];
			shapePossiblyChosen = 0;
		}
		else {
			lastHoveredShape = -1;
			shapePossiblyChosen = -1;
		}
	}
	else {
		lastHoveredShape = -1;
		shapePossiblyChosen = -1;
	}
}

void Navigator::PrintShapes() {
	
	// Print all shapes in the currently selected Layer
	LOG_WARN("Layer #{}: Name '{}'", file.GetActiveLayer()->layerID.Get(), file.GetActiveLayer()->name);
	for (Shape& shape : file.GetActiveLayerShapes()) {
		LOG_WARN("Shape #{}: ", shape.shapeID.Get());
	}
}









void Navigator::OnKeyPressed(Battery::KeyPressedEvent* event) {
	
	switch (event->keycode) {

	case ALLEGRO_KEY_TAB:
		// Switch through all possibly selected shapes, is wrapped around automatically
		shapePossiblyChosen++;
		UpdateHoveredShapes();
		break;

	case ALLEGRO_KEY_DELETE:
		// Delete selected shapes
		for (ShapeID id : selectedShapes) {
			file.GetActiveLayer()->RemoveShape(id);
		}
		selectedShapes.clear();
		break;

	case ALLEGRO_KEY_LEFT:
		// Move shape to the left by one unit
		for (ShapeID id : selectedShapes) {
			file.GetActiveLayer()->MoveShapeLeft(id, snapSize);
		}
		break;

	case ALLEGRO_KEY_RIGHT:
		// Move shape to the right by one unit
		for (ShapeID id : selectedShapes) {
			file.GetActiveLayer()->MoveShapeRight(id, snapSize);
		}
		break;

	case ALLEGRO_KEY_UP:
		// Move shape up by one unit
		for (ShapeID id : selectedShapes) {
			file.GetActiveLayer()->MoveShapeUp(id, snapSize);
		}
		break;

	case ALLEGRO_KEY_DOWN:
		// Move shape down by one unit
		for (ShapeID id : selectedShapes) {
			file.GetActiveLayer()->MoveShapeDown(id, snapSize);
		}
		break;

	case ALLEGRO_KEY_Z:
		// Undo previous action
		if (controlKeyPressed) {
			file.GetActiveLayer()->UndoPreviousAction();
		}
		break;

	default:
		break;

	}
}

void Navigator::OnMouseLeftClicked(const glm::vec2& position, const glm::vec2& snapped) {

	if (lastHoveredShape.Get() == -1) {
		OnSpaceClicked(position, snapped);
	}
	else {
		OnShapeClicked(position, snapped);
	}
}

void Navigator::OnMouseRightClicked(const glm::vec2& position, const glm::vec2& snapped) {

	if (selectedTool == CursorTool::LINE || selectedTool == CursorTool::LINE_STRIP) {
		CancelShape();
	}
}

void Navigator::OnMouseWheelClicked(const glm::vec2& position, const glm::vec2& snapped) {


}

void Navigator::OnMouseReleased(const glm::vec2& position) {

	if (selectionBoxActive) {
		selectionBoxActive = false;

		// If CTRL key is pressed, don't delete previously selected shapes
		if (!controlKeyPressed) {
			selectedShapes.clear();
		}

		// Select all shapes inside the selection box
		for (Shape& shape : file.GetActiveLayer()->GetShapes()) {
			if (shape.IsInSelectionBoundary(selectionBoxPointA, selectionBoxPointB)) {
				SelectShape(shape.shapeID);
			}
		}
	}

}

void Navigator::OnMouseMoved(const glm::vec2& position, const glm::vec2& snapped) {
	using namespace Battery;

	if (applicationPointer->window.GetLeftMouseButton()) {
		OnMouseDragged(position, snapped);
	}
	else {
		OnMouseHovered(position, snapped);
	}

}

void Navigator::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped) {

	previewPointPosition = mouseSnapped;
	previewShapePoint2 = snapped;
	selectionBoxActive = false;

	UpdateHoveredShapes();

}

void Navigator::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped) {

	selectionBoxPointB = position;

}

void Navigator::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped) {

	switch (selectedTool) {
	
	case CursorTool::SELECT:
		// Start selection box
		selectionBoxActive = true;
		selectionBoxPointA = position;
		selectionBoxPointB = position;
		break;
	
	case CursorTool::LINE:	// Here a line will be started or finished
		if (previewShape == ShapeType::INVALID) {	// No shape was started, start one
			previewShape = ShapeType::LINE;
			previewPointShown = false;
			previewShapePoint1 = snapped;
			previewShapePoint2 = snapped;
			LOG_TRACE(__FUNCTION__"(): Started a line preview");
		}
		else {
			if (previewShape == ShapeType::LINE) {	// A Line is already drawn, finish it
				AddLine(previewShapePoint1, snapped);
			}
			else {	// Another shape is drawn, this is invalid
				LOG_ERROR(__FUNCTION__ "(): Error while finishing line shape: A non-line shape is selected!");
			}
			previewShape = ShapeType::INVALID;
			previewPointShown = true;
		}
		break;

	case CursorTool::LINE_STRIP:	// Here a line will be started or continued
		if (previewShape == ShapeType::INVALID) {	// No shape was started, start one
			previewShape = ShapeType::LINE;
			previewPointShown = false;
			previewShapePoint1 = snapped;
			previewShapePoint2 = snapped;
			LOG_TRACE(__FUNCTION__"(): Started a line preview");
		}
		else {
			if (previewShape == ShapeType::LINE) {	// A Line is already drawn, finish and continue it
				AddLine(previewShapePoint1, snapped);
				previewPointShown = false;
				previewShapePoint1 = snapped;
				previewShapePoint2 = snapped;
				LOG_TRACE(__FUNCTION__"(): Line was drawn and continued");
			}
			else {	// Another shape is drawn, this is invalid
				previewPointShown = true;
				LOG_ERROR(__FUNCTION__ "(): Error while finishing line shape: A non-line shape is selected!");
			}
		}
		break;
	
	default:
		break;
	}

}

void Navigator::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped) {

	switch (selectedTool) {

	case CursorTool::SELECT:
		// Select shapes being clicked

		if (!controlKeyPressed) {
			selectedShapes.clear();
		}

		if (IsShapeSelected(lastHoveredShape)) {
			UnselectShape(lastHoveredShape);
		}
		else {
			SelectShape(lastHoveredShape);
		}
		break;

	default:
		break;
	}

}

void Navigator::OnToolChanged() {

	CancelShape();

	// Various tools
	switch (selectedTool) {

		// Selection tool
	case CursorTool::SELECT:
		previewPointShown = false;
		previewPointPosition = mouseSnapped;
		selectionBoxActive = false;
		break;

	case CursorTool::LINE:
		previewPointShown = true;
		previewPointPosition = mouseSnapped;
		selectionBoxActive = false;
		break;

	case CursorTool::LINE_STRIP:
		previewPointShown = true;
		previewPointPosition = mouseSnapped;
		selectionBoxActive = false;
		break;

	default:
		break;
	}

}






void Navigator::AddLine(const glm::vec2& p1, const glm::vec2& p2) {

	// Safety check
	if (p1 == p2) {
		LOG_WARN(__FUNCTION__ "(): Line is not added to buffer: Start and end points are identical!");
		return;
	}

	LOG_TRACE(__FUNCTION__"(): Line was added");

	file.AddShape(ShapeType::LINE, p1, p2, currentLineThickness, currentShapeColor);
	file.FileChanged();

}

































void Navigator::DrawGrid() {
	using namespace Battery;

	float thickness = gridLineWidth;
	float alpha = min(scale * gridAlphaFactor + gridAlphaOffset, gridAlphaMax);
	glm::vec4 color = glm::vec4(gridLineColor, gridLineColor, gridLineColor, alpha);

	int w = applicationPointer->window.GetWidth();
	int h = applicationPointer->window.GetHeight();

	// Sub grid lines
	//if (scale * snapSize > 3) {
		for (float x = panOffset.x + w / 2; x < w; x += scale * snapSize) {
			Renderer2D::DrawPrimitiveLine({ x, 0 }, { x, h }, thickness, color);
		}
		for (float x = panOffset.x + w / 2; x > 0; x -= scale * snapSize) {
			Renderer2D::DrawPrimitiveLine({ x, 0 }, { x, h }, thickness, color);
		}
		for (float y = panOffset.y + h / 2; y < h; y += scale * snapSize) {
			Renderer2D::DrawPrimitiveLine({ 0, y }, { w, y }, thickness, color);
		}
		for (float y = panOffset.y + h / 2; y > 0; y -= scale * snapSize) {
			Renderer2D::DrawPrimitiveLine({ 0, y }, { w, y }, thickness, color);
		}
	//}
}

void Navigator::RenderShapes() {
	using namespace Battery;
	
	std::vector<::Layer>& layers = file.GetLayers();
	for (size_t i = 0; i < layers.size(); i++) {
		::Layer& layer = layers[i];

		for (::Shape& shape : layer.GetShapes()) {

			// Skip the shape if it's not on the screen
			auto pair = shape.GetBoundingBox();
			glm::vec2 min = ConvertWorkspaceToScreenCoords(pair.first);
			glm::vec2 max = ConvertWorkspaceToScreenCoords(pair.second);
			float w = applicationPointer->window.GetWidth();
			float h = applicationPointer->window.GetHeight();

			if ((min.x < 0 && max.x < 0) ||
				(min.x > w && max.x > w) ||
				(min.y < 0 && max.y < 0) ||
				(min.y > h && max.y > h)
			) {
				LOG_TRACE(__FUNCTION__ "(): Skipping rendering shape #{}: Bounding box is not on screen", shape.shapeID.Get());
				continue;
			}

			// Render the shape
			switch (shape.type) {

			case ShapeType::LINE:
				if (layer.layerID == file.GetActiveLayerID()) {

					// Shape is selected
					if (IsShapeSelected(shape.shapeID)) {
						if (shape.shapeID == lastHoveredShape) {	// Shape is selected and hovered
							DrawLine(shape.p1, shape.p2, shape.thickness, 
								(hoveredLineColor + selectedLineColor) / 2.f);
						}
						else {
							DrawLine(shape.p1, shape.p2, shape.thickness, selectedLineColor);
						}
					}
					else { // Shape is simply hovered
						if (shape.shapeID == lastHoveredShape) {
							DrawLine(shape.p1, shape.p2, shape.thickness, hoveredLineColor);
						}
						else {
							DrawLine(shape.p1, shape.p2, shape.thickness, normalLineColor);
						}
					}
				}
				else {
					DrawLine(shape.p1, shape.p2, shape.thickness, disabledLineColor);
				}
				break;

			//case SHAPE_CIRCLE:
			//	break;

			default:
				break;
			}
		}
	}
}

void Navigator::DrawLine(const glm::vec2& p1, const glm::vec2& p2, float thickness, const glm::vec4 color) {
	glm::vec2 c1 = ConvertWorkspaceToScreenCoords(p1);
	glm::vec2 c2 = ConvertWorkspaceToScreenCoords(p2);
	Battery::Renderer2D::DrawLine(c1, c2, thickness * scale, color);
}
