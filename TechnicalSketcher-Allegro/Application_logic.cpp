
#include "pch.h"
#include "Application.h"
#include "utils.h"
#include "math_geometry.h"




float dist(glm::vec2 a, glm::vec2 b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

glm::vec2 round(glm::vec2 v) {
	return glm::vec2(roundf(v.x), roundf(v.y));
}



void Application::updateMousePositions() {

	// Convert mouse positions and save
	mouse_workspace = convertScreenToWorkspaceCoords({ mouse.x, mouse.y });
	mouseSnapped_workspace = round(mouse_workspace / snapSize) * snapSize;

	if (getKey(ALLEGRO_KEY_LCTRL)) {	// If ctrl is pressed, enable smooth mode
		mouseSnapped_workspace = mouse_workspace;
	}
}

glm::vec2 Application::convertScreenToWorkspaceCoords(glm::vec2 v) {
	return (v - panOffset - glm::vec2(width, height) * 0.5f) / scale;
}

glm::vec2 Application::convertWorkspaceToScreenCoords(glm::vec2 v) {
	return panOffset + v * scale + glm::vec2(width, height) * 0.5f;
}





void Application::generateLayerPreviews() {
	timestamp_t start = micros();

	for (Layer& layer : layers.getLayers()) {
		ALLEGRO_BITMAP* bitmap = createLayerPreviewBitmap(layer.layerID, GUI_PREVIEWWINDOW_SIZE, GUI_PREVIEWWINDOW_SIZE);
		layer.cloneFrom(bitmap);
		al_destroy_bitmap(bitmap);
	}
	previewRegenerateFlag = false;
	//std::cout << "Generated preview bitmaps, took " << (micros() - start) / 1000.0 << " ms" << std::endl;
}

void Application::addLayer() {
	addLayer("Layer #" + std::to_string(maxLayers));
	previewRegenerateFlag = true;
}

void Application::addLayer(const std::string& name) {

	cancelShape();

	layers.addLayerFront(name);
	maxLayers++;					// Just for the name, not critical

	previewRegenerateFlag = true;
}

void Application::addLine(glm::vec2 p1, glm::vec2 p2) {

	if (layers.getSelectedLayerID() == -1) {
		std::cout << "WARNING: Can't draw, no layer selected!" << std::endl;
		return;
	}

	if (p1 != p2) {
		layers.getSelectedLayer().addShape(SHAPE_LINE, p1, p2, ctrl_currentLineThickness);
	}

	previewRegenerateFlag = true;
}

void Application::changeMode(int mode) {

	cancelShape();
	selectedShapes.clear();

	switch (mode) {

	case TOOL_SELECT:
		selectedTool = TOOL_SELECT;
		showPreviewPoint = false;
		drawingSelectionBox = false;
		break;

	case TOOL_LINE:
		selectedTool = TOOL_LINE;
		previewPoint = mouseSnapped_workspace;
		showPreviewPoint = true;
		drawingLine = false;
		drawingSelectionBox = false;
		break;

	case TOOL_LINE_STRIP:
		selectedTool = TOOL_LINE_STRIP;
		previewPoint = mouseSnapped_workspace;
		showPreviewPoint = true;
		drawingLine = false;
		drawingSelectionBox = false;
		break;

	default:
		break;
	}
}

bool Application::isShapeSelected(ShapeID shape) {

	for (size_t i = 0; i < selectedShapes.size(); i++) {
		if (selectedShapes[i] == shape) {
			return true;
		}
	}

	return false;
}

bool Application::deleteShape(ShapeID shape) {
	previewRegenerateFlag = true;
	return layers.getSelectedLayer().removeShape(shape);
}

void Application::cancelShape() {

	if (selectedTool != TOOL_SELECT)
		showPreviewPoint = true;
	else
		showPreviewPoint = false;

	drawingLine = false;
}

void Application::prepareGUI() {

	if (isMouseOnGui && previewRegenerateFlag) {
		generateLayerPreviews();
		previewRegenerateFlag = false;
	}
}








std::vector<ShapeID> Application::getHoveredShapes() {

	std::vector<ShapeID> shapes;

	if (layers.getSelectedLayerID() == -1)
		return shapes;

	for (Shape& shape : layers.getSelectedLayer().getShapes()) {

		if (shape.type == SHAPE_LINE) {

			glm::vec2 p1 = convertWorkspaceToScreenCoords(shape.p1);
			glm::vec2 p2 = convertWorkspaceToScreenCoords(shape.p2);

			if (getDistanceAroundLine(p1, p2, mouse) <= highlightDistanceToMouse) {
				shapes.push_back(shape.shapeID);
			}
		}
	}

	return shapes;
}

ShapeID Application::getClosestHoveredShape() {

	if (layers.getSelectedLayerID() == -1)
		return false;

	ShapeID closestID = -1;
	float closest = -1;

	for (Shape& shape : layers.getSelectedLayer().getShapes()) {

		if (shape.type == SHAPE_LINE) {

			glm::vec2 p1 = convertWorkspaceToScreenCoords(shape.p1);
			glm::vec2 p2 = convertWorkspaceToScreenCoords(shape.p2);

			float distance = getDistanceAroundLine(p1, p2, mouse);

			if (distance <= highlightDistanceToMouse && (distance < closest || closest == -1)) {
				closest = distance;
				closestID = shape.shapeID;
			}
		}
	}

	return closestID;
}







void Application::mouseMoved() {

	switch (selectedTool) {

	case TOOL_SELECT:

		// Choose closest hovered shape
		mouseOnShape = getClosestHoveredShape();
		break;

	case TOOL_LINE:
		break;


	case TOOL_LINE_STRIP:
		break;

	default:
		std::cout << "Invalid tool selected: " << selectedTool << std::endl;
		break;
	}
}

void Application::mouseHovered() {

	switch (selectedTool) {

	case TOOL_SELECT:

		hoveredShape = getClosestHoveredShape();

		// Stop drawing selection box
		if (drawingSelectionBox) {
			drawingSelectionBox = false;
			selectedShapes.clear();

			float left = std::min(previewLineStart.x, mouse_workspace.x);
			float right = std::max(previewLineStart.x, mouse_workspace.x);
			float top = std::max(previewLineStart.y, mouse_workspace.y);
			float bottom = std::min(previewLineStart.y, mouse_workspace.y);

			for (Shape& shape : layers.getSelectedLayer().getShapes()) {
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

		break;


	case TOOL_LINE:

		if (!drawingLine) {
			previewPoint = mouseSnapped_workspace;
		}
		break;


	case TOOL_LINE_STRIP:

		if (!drawingLine) {
			previewPoint = mouseSnapped_workspace;
		}
		break;

	default:
		std::cout << "Invalid tool selected: " << selectedTool << std::endl;
		break;
	}
}

void Application::mouseDragged() {

	if (selectedTool == TOOL_SELECT) {
		if (mouseWheelPressed) {
			panOffset.x += mouse.x - pmouse.x;
			panOffset.y += mouse.y - pmouse.y;
		}
	}

	// Un-hover shape when mouse is dragged off
	if (mouseOnShape != hoveredShape) {
		hoveredShape = -1;
	}
}











// Selection tool, a shape was clicked
void Application::selectToolShapeClicked(ShapeID shape, bool ctrlKey) {
	//std::cout << "Shape #" << shape << " clicked" << std::endl;
}

//// Selection tool, a point of a shape was clicked
//void Application::selectToolShapePointClicked(ShapeID shape, int pointNumber) {
//	std::cout << "Point #" << pointNumber << " of Shape #" << shape << " clicked" << std::endl;
//}

// Selection tool, the blank space was clicked
void Application::selectToolSpaceClicked(bool ctrlKey) {

	drawingSelectionBox = true;
	showPreviewPoint = false;
	previewLineStart = mouse_workspace;

	//std::cout << "Selection Space clicked" << std::endl;
}

// Selection tool, right clicked
void Application::selectToolRightClicked() {
	//std::cout << "Selection Right clicked" << std::endl;
}

// Selection tool, a shape was released
void Application::selectToolShapeReleased(ShapeID shape, bool ctrlKey) {

	// Quickly update shape if mouse has not moved
	mouseOnShape = getClosestHoveredShape();

	// Only do something if released shape is the same 
	// as the one that was clicked in the first place

	if (shape == hoveredShape) {
		if (ctrlKey) {		// Shape was ctrl-clicked

			bool existed = false;
			for (size_t i = 0; i < selectedShapes.size(); i++) {	// Go through all selected shapes
				if (selectedShapes[i] == shape) {
					selectedShapes.erase(selectedShapes.begin() + i);	// Shape was selected and is un-selected
					existed = true;
					break;
				}
			}

			if (!existed) {		// Shape was not selected yet and is added
				selectedShapes.push_back(shape);
			}
		}
		else {				// Shape was clicked normally
			selectedShapes.clear();
			selectedShapes.push_back(shape);
		}
	}

	//std::cout << "Shape #" << shape << " released" << std::endl;
}

//// Selection tool, a point of a shape was released
//void Application::selectToolShapePointReleased(ShapeID shape, int pointNumber) {
//	std::cout << "Point #" << pointNumber << " of Shape #" << shape << " released" << std::endl;
//}

// Selection tool, the blank space was released
void Application::selectToolSpaceReleased(bool ctrlKey) {

	// Unselect all shapes, but only if ctrl is not pressed
	// to avoid frustration when selecting shapes
	if (!ctrlKey && hoveredShape == -1) {
		selectedShapes.clear();
	}

	//std::cout << "Selection Space released" << std::endl;
}

// Selection tool, right mouse button released
void Application::selectToolMouseRightReleased() {
	//std::cout << "Selection mouse right released" << std::endl;
}




// Line tool, space was clicked
void Application::lineToolSpaceClicked() {

	if (!drawingLine) {		// If no line yet, start one
		drawingLine = true;
		showPreviewPoint = false;
		previewLineStart = mouseSnapped_workspace;
	}
	else {						// If line is being drawn, end it here
		showPreviewPoint = true;
		drawingLine = false;
		previewPoint = mouseSnapped_workspace;
		addLine(mouseSnapped_workspace, previewLineStart);
	}

	//std::cout << "Line Space clicked" << std::endl;
}

// Line tool, right clicked
void Application::lineToolRightClicked() {

	cancelShape();

	//std::cout << "Line tool, Right clicked" << std::endl;
}

// Line tool, left mouse button released
void Application::lineToolMouseReleased() {
	//std::cout << "Line mode mouse released" << std::endl;
}

// Line tool, right mouse button released
void Application::lineToolMouseRightReleased() {
	//std::cout << "Line mode mouse right released" << std::endl;
}




// Line strip tool, space was clicked
void Application::lineStripToolSpaceClicked() {

	if (!drawingLine) {		// If no line yet, start one
		drawingLine = true;
		showPreviewPoint = false;
		previewLineStart = mouseSnapped_workspace;
	}
	else {						// If line is being drawn, end it and start a new one
		showPreviewPoint = false;
		drawingLine = true;
		addLine(mouseSnapped_workspace, previewLineStart);
		previewLineStart = mouseSnapped_workspace;
	}

	//std::cout << "Line strip Space clicked" << std::endl;
}

// Line strip tool, right clicked
void Application::lineStripToolRightClicked() {

	cancelShape();

	//std::cout << "Line strip, Right clicked" << std::endl;
}

// Line strip tool, left mouse button released
void Application::lineStripToolMouseReleased() {
	//std::cout << "Line strip mode mouse released" << std::endl;
}

// Line strip tool, right mouse button released
void Application::lineStripToolMouseRightReleased() {
	//std::cout << "Line strip mode mouse right released" << std::endl;
}
