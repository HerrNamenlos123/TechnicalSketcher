
#include "pch.h"
#include "MainEngine.h"
#include "math_geometry.h"


void MainEngine::handleEvents() {

	ImGuiIO& io = ImGui::GetIO();
	gui_mouseOnGui = io.WantCaptureMouse;

	// Handle swapping layer order
	if (events_moveLayerBackID != -1) {
		layers.moveLayerBack(events_moveLayerBackID);
		events_moveLayerBackID = -1;
	}
	if (events_moveLayerFrontID != -1) {
		layers.moveLayerFront(events_moveLayerFrontID);
		events_moveLayerFrontID = -1;
	}

	// Handle Event when a layer is clicked
	if (events_layerSelectedEventFlag) {
		OnLayerSelect(layers.findLayer(layers.selectedLayer));
		events_layerSelectedEventFlag = false;
	}

	// Handle Event when the mouse enters the layer window
	if (events_layerWindowMouseEnteredEventFlag) {
		OnMouseEnteredLayerWindow();
		events_layerWindowMouseEnteredEventFlag = false;
	}


	// Mouse events

	mousePos = glm::vec2((mouse.x - panOffset.x - width / 2.f) / scale, (mouse.y - panOffset.y - height / 2.f) / scale);
	mouseSnapped = glm::vec2(round(mousePos.x / ctrl_snapTo) * ctrl_snapTo, round(mousePos.y / ctrl_snapTo) * ctrl_snapTo);
	
	// Left mouse button pressed
	if (mouseButtonLeftPressed && !pmouseButtonLeftPressed) {
		OnMouseButtonLeftClicked();
	}

	// Right mouse button pressed
	if (mouseButtonRightPressed && !pmouseButtonRightPressed) {
		OnMouseButtonRightClicked();
	}

	// Left mouse button released
	if (!mouseButtonLeftPressed && pmouseButtonLeftPressed) {
		OnMouseButtonLeftReleased();
	}

	// Right mouse button released
	if (!mouseButtonRightPressed && pmouseButtonRightPressed) {
		OnMouseButtonRightReleased();
	}

	// Left mouse button pressed
	if (mousePressed && pmouse != mouse) {
		OnMouseDragged();
	}

	// Left mouse button pressed
	if (!mousePressed && pmouse != mouse) {
		OnMouseHovered();
	}
}









void MainEngine::OnMouseButtonLeftClicked() {

	if (gui_mouseOnGui)
		return;

	if (cursorMode == MODE_LINE || cursorMode == MODE_LINE_STRIP) {

		if (!draggingLine) {
			draggingLine = true;
			showPreviewPoint = false;
			previewLineStart = mouseSnapped;
		}
		else {

			if (cursorMode == MODE_LINE_STRIP) {
				if (mouseSnapped != previewLineStart) {
					addLine(mouseSnapped, previewLineStart);
				}
				showPreviewPoint = false;
				draggingLine = true;
				previewLineStart = mouseSnapped;
			}
			else {
				showPreviewPoint = true;
				draggingLine = false;
				previewPoint = mouseSnapped;
				if (mouseSnapped != previewLineStart) {
					addLine(mouseSnapped, previewLineStart);
				}
			}

		}
	}
	else if (cursorMode == MODE_SELECT) {

		bool ctrl = getKey(ALLEGRO_KEY_LCTRL);

		if (hoveredShape == -1) {
			if (!ctrl) {	// Prevent unselecting everything when missing the line
				selectedShapes.clear();

				// Start drawing selection box
				draggingSelectionBox = true;
				previewPoint = mouse;
				showPreviewPoint = false;
			}
		}
		else {

			if (ctrl) {	// CTRL Key is held

				if (isShapeSelected(hoveredShape)) {	// If already selected, unselect

					// Remove from the list
					for (size_t i = 0; i < selectedShapes.size(); i++) {
						if (selectedShapes[i] == hoveredShape) {
							selectedShapes.erase(selectedShapes.begin() + i);
						}
					}
				}
				else {
					selectedShapes.push_back(hoveredShape);
				}
			}
			else {
				selectedShapes.clear();
				selectedShapes.push_back(hoveredShape);
			}
		}
	}

	std::cout << "Left mouse clicked" << std::endl;
}

void MainEngine::OnMouseButtonRightClicked() {

	cancelShape();

	std::cout << "Right mouse clicked" << std::endl;
}

void MainEngine::OnMouseButtonLeftReleased() {

	// Not caring when mouse is on UI

	// Stop drawing selection box
	if (draggingSelectionBox) {
		draggingSelectionBox = false;
		selectedShapes.clear();

		glm::vec2 prev = (previewPoint - glm::vec2(width, height) * 0.5f - panOffset) / scale;

		float left = std::min(prev.x, mousePos.x);
		float right = std::max(prev.x, mousePos.x);
		float top = std::max(prev.y, mousePos.y);
		float bottom = std::min(prev.y, mousePos.y);

		for (Shape& shape : layers.findLayer(layers.selectedLayer).getShapes()) {
			if (shape.type == SHAPE_LINE) {
				if (shape.p1.x >= left && shape.p1.x <= right && shape.p1.y >= bottom && shape.p1.y <= top &&
					shape.p2.x >= left && shape.p2.x <= right && shape.p2.y >= bottom && shape.p2.y <= top)
				{
					// Line is fully contained in the selection box
					selectedShapes.push_back(shape.shapeID);
				}
			}
		}
	}

	std::cout << "Left mouse released" << std::endl;
}

void MainEngine::OnMouseButtonRightReleased() {

	if (gui_mouseOnGui)
		return;

	std::cout << "Right mouse released" << std::endl;
}

void MainEngine::OnMouseDragged() {

	if (gui_mouseOnGui)
		return;

	if (cursorMode == MODE_SELECT && mouseWheelPressed) {
		panOffset.x += mouse.x - pmouse.x;
		panOffset.y += mouse.y - pmouse.y;
	}

	std::cout << "Mouse dragged" << std::endl;
}

void MainEngine::OnMouseHovered() {

	if (gui_mouseOnGui)
		return;

	draggingSelectionBox = false;

	if (cursorMode == MODE_LINE || cursorMode == MODE_LINE_STRIP) {
		if (!draggingLine) {
			previewPoint = mouseSnapped;
		}
	}
	else if (cursorMode == MODE_SELECT) {

		hoveredShape = -1;
		Layer& layer = layers.findLayer(layers.selectedLayer);

		if (layer.size() > 0) {
			for (int i = static_cast<int>(layer.size()) - 1; i >= 0; i--) {
				Shape& shape = layer.getShapes()[i];

				glm::vec2 p1 = panOffset + shape.p1 * scale + glm::vec2(width, height) * 0.5f;
				glm::vec2 p2 = panOffset + shape.p2 * scale + glm::vec2(width, height) * 0.5f;

				if (shape.type == SHAPE_LINE) {
					if (getDistanceAroundLine(p1, p2, mouse) <= ctrl_highlightCursorThreshold) {
						hoveredShape = shape.shapeID;
						break;
					}
				}
			}
		}
	}

	std::cout << "Mouse hovered" << std::endl;
}

void MainEngine::OnLayerSelect(const Layer& layer) {

	cancelShape();

	std::cout << "OnLayerSelect" << std::endl;
}

void MainEngine::OnMouseEnteredLayerWindow() {

	if (gui_previewChanged) {
		gui_previewChanged = false;
		generateLayerPreviews();
	}

	std::cout << "OnMouseEnteredLayerWindow" << std::endl;
}
