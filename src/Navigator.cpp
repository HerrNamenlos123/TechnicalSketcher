
#include "Navigator.h"
#include "Layer.h"
#include "config.h"

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

	// Allow smooth positioning when CTRL is pressed
	if (applicationPointer->GetKey(ALLEGRO_KEY_LCTRL)) {
		mouseSnapped = mousePosition;
	}
	
	if (scrollBuffer != 0.f) {
		MouseScrolled(scrollBuffer);
		scrollBuffer = 0;
	}

	UpdateEvents();
	UpdateShapes();
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

	// Draw second part of the selection box
	if (selectionBoxActive) {			// Outline in the front
		glm::vec2 p1 = ConvertWorkspaceToScreenCoords(selectionBoxPointA);
		glm::vec2 p2 = ConvertWorkspaceToScreenCoords(selectionBoxPointB);
		Renderer2D::DrawRectangle(p1, p2, 2, selectionBoxOutlineColor, { 0, 0, 0, 0 }, 0);
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

	for (Battery::MouseButtonPressedEvent event : mousePressedEventBuffer) {
		glm::vec2 position = ConvertScreenToWorkspaceCoords({ event.x, event.y });
		glm::vec2 snapped = round(position / snapSize) * snapSize;

		// Allow smooth positioning when CTRL is pressed
		if (applicationPointer->GetKey(ALLEGRO_KEY_LCTRL)) {
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

	for (Battery::MouseButtonReleasedEvent event : mouseReleasedEventBuffer) {
		glm::vec2 position = ConvertScreenToWorkspaceCoords({ event.x, event.y });
		glm::vec2 snapped = round(position / snapSize) * snapSize;

		// Allow smooth positioning when CTRL is pressed
		if (applicationPointer->GetKey(ALLEGRO_KEY_LCTRL)) {
			snapped = position;
		}

		OnMouseReleased(position);
	}
	mouseReleasedEventBuffer.clear();

	for (Battery::MouseMovedEvent event : mouseMovedEventBuffer) {
		glm::vec2 position = ConvertScreenToWorkspaceCoords({ event.x, event.y });
		glm::vec2 snapped = round(position / snapSize) * snapSize;
		
		// Allow smooth positioning when CTRL is pressed
		if (applicationPointer->GetKey(ALLEGRO_KEY_LCTRL)) {
			snapped = position;
		}

		OnMouseMoved(position, snapped);
	}
	mouseMovedEventBuffer.clear();
}

void Navigator::UpdateShapes() {


}

void Navigator::CancelShape() {

	LOG_ERROR("CANCEL NOW");
}

void Navigator::UseTool(enum class CursorTool tool) {
	selectedTool = tool;
	OnToolChanged();
}






void Navigator::OnMouseLeftClicked(const glm::vec2& position, const glm::vec2& snapped) {

	if (hoveredShape.Get() == -1) {
		OnSpaceClicked(position, snapped);
	}
	else {
		OnShapeClicked(position, snapped);
	}
}

void Navigator::OnMouseRightClicked(const glm::vec2& position, const glm::vec2& snapped) {


}

void Navigator::OnMouseWheelClicked(const glm::vec2& position, const glm::vec2& snapped) {


}

void Navigator::OnMouseReleased(const glm::vec2& position) {

	CancelShape();

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

}

void Navigator::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped) {


}

void Navigator::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped) {

	switch (selectedTool) {
	
	case CursorTool::SELECT:
		// Nothing happens when clicking void
		break;
	
	case CursorTool::LINE:	// Here a line will be started or finished
		if (previewShape == ShapeType::INVALID) {	// No shape was started, start one
			previewShape = ShapeType::LINE;
			previewShapePoint1 = snapped;
			previewShapePoint2 = snapped;
			LOG_TRACE(__FUNCTION__"(): Started a line preview");
		}
		else {
			if (previewShape == ShapeType::LINE) {	// A Line is already drawn, finish it
				AddLine(previewShapePoint1, snapped);
				LOG_TRACE(__FUNCTION__"(): Line was added");
			}
			else {	// Another shape is drawn, this is invalid
				LOG_ERROR(__FUNCTION__ "(): Error while finishing line shape: A non-line shape is selected!");
			}
			previewShape = ShapeType::INVALID;
		}
		break;
	
	default:
		break;
	}

}

void Navigator::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped) {


}

void Navigator::OnToolChanged() {

	CancelShape();

	// Various tools
	switch (selectedTool) {

		// Selection tool
	case CursorTool::SELECT:
		previewPointShown = true;
		break;

	case CursorTool::LINE:
		previewPointShown = false;

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

	file.AddShape(ShapeType::LINE, p1, p2, currentLineThickness);

}

































void Navigator::DrawGrid() {
	using namespace Battery;

	float thickness = gridLineWidth;
	glm::vec4 color = glm::vec4(gridLineColor, gridLineColor, gridLineColor, 255);

	int w = applicationPointer->window.GetWidth();
	int h = applicationPointer->window.GetHeight();

	// Sub grid lines
	if (scale * snapSize > 3) {
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
	}
}

void Navigator::RenderShapes() {
	using namespace Battery;
	
	for (::Layer* layer : file.GetLayersReverse()) {
		for (::Shape* shape : layer->GetShapes()) {

			switch (shape->type) {

			case ShapeType::LINE:
				if (layer->layerID == file.GetActiveLayerID()) {

					// Shape is selected
					if (IsShapeSelected(shape->shapeID)) {
						if (shape->shapeID == lastHoveredShape) {	// Shape is selected and hovered
							DrawLine(shape->p1, shape->p2, shape->thickness, 
								(hoveredLineColor + selectedLineColor) / 2.f);
						}
						else {
							DrawLine(shape->p1, shape->p2, shape->thickness, selectedLineColor);
						}
					}
					else { // Shape is simply hovered
						if (shape->shapeID == lastHoveredShape) {
							DrawLine(shape->p1, shape->p2, shape->thickness, hoveredLineColor);
						}
						else {
							DrawLine(shape->p1, shape->p2, shape->thickness, normalLineColor);
						}
					}
				}
				else {
					DrawLine(shape->p1, shape->p2, shape->thickness, disabledLineColor);
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
	Battery::Renderer2D::DrawLine(c1, c2, thickness, color);
}
