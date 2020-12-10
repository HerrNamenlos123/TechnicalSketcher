
#include "pch.h"
#include "Application.h"
#include "math_geometry.h"


void Application::handleEvents() {


	// Handle control events from the LayerWindow GUI to the application

	if (layerWindow.moveLayerBackFlag) {
		layers.moveLayerBack(layerWindow.moveLayerBackID);
		layerWindow.moveLayerBackFlag = false;
	}
	if (layerWindow.moveLayerFrontFlag) {
		layers.moveLayerFront(layerWindow.moveLayerFrontID);
		layerWindow.moveLayerFrontFlag = false;
	}
	if (layerWindow.layerSelectedFlag) {
		layers.selectLayer(layerWindow.selectedLayer);
		OnLayerSelect();
		layerWindow.layerSelectedFlag = false;
	}
	if (layerWindow.addLayerFlag) {
		addLayer();
		generateLayerPreviews();
		layerWindow.addLayerFlag = false;
	}
	if (layerWindow.mouseEnteredWindowFlag) {
		OnMouseEnteredLayerWindow();
		layerWindow.mouseEnteredWindowFlag = false;
	}



	// Handle events from and to the Toolbox GUI

	if (toolboxWindow.changeToolFlag) {
		changeMode(toolboxWindow.clickedTool);
		toolboxWindow.changeToolFlag = false;
	}
	toolboxWindow.selectedTool = selectedTool;



	// Mouse events

	isMouseOnGui = ribbonWindow.isMouseOnWindow |
				   layerWindow.isMouseOnWindow |
				   toolboxWindow.isMouseOnWindow |
				   mouseInfoWindow.isMouseOnWindow;
	
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

	// Mouse dragged
	if (mousePressed && pmouse != mouse) {
		OnMouseDragged();
	}

	// Mouse hovered
	if (!mousePressed && pmouse != mouse) {
		OnMouseHovered();
	}

	// Mouse moved
	if (pmouse != mouse) {
		OnMouseMoved();
	}
}









void Application::OnMouseButtonLeftClicked() {

	if (isMouseOnGui)	// If mouse is on GUI overlay, ignore any inputs
		return;

	switch (selectedTool) {

	case TOOL_SELECT:

		if (mouseOnShape != -1) {		// A Shape was clicked
			selectToolShapeClicked(mouseOnShape, getKey(ALLEGRO_KEY_LCTRL));
		}
		else {								// Open space was clicked
			selectToolSpaceClicked(getKey(ALLEGRO_KEY_LCTRL));
		}
		break;

	case TOOL_LINE:			// If a line is to be drawn
								
		// Open space was clicked
		lineToolSpaceClicked();

		break;

	case TOOL_LINE_STRIP:

		// Open space was clicked
		lineStripToolSpaceClicked();

		break;

	default:
		std::cout << "Invalid tool selected: " << selectedTool << std::endl;
		break;
	}

	//std::cout << "Left mouse clicked" << std::endl;
}

void Application::OnMouseButtonRightClicked() {

	if (isMouseOnGui)	// If mouse is on GUI overlay, ignore any inputs
		return;

	switch (selectedTool) {

	case TOOL_SELECT:
		selectToolRightClicked();
		break;

	case TOOL_LINE:
		lineToolRightClicked();
		break;

	case TOOL_LINE_STRIP:
		lineStripToolRightClicked();
		break;

	default:
		std::cout << "Invalid tool selected: " << selectedTool << std::endl;
		break;
	}

	//std::cout << "Right mouse clicked" << std::endl;
}

void Application::OnMouseButtonLeftReleased() {

	//if (isMouseOnGui)	// If mouse is on GUI overlay, ignore any inputs
	//	return;

	switch (selectedTool) {

	case TOOL_SELECT:
		
		if (mouseOnShape != -1) {			// A Shape was clicked
			selectToolShapeReleased(mouseOnShape, getKey(ALLEGRO_KEY_LCTRL));
		}
		else {								// Open space was clicked
			selectToolSpaceReleased(getKey(ALLEGRO_KEY_LCTRL));
		}
		break;

	case TOOL_LINE:
		lineToolMouseReleased();
		break;

	case TOOL_LINE_STRIP:
		lineStripToolMouseReleased();
		break;

	default:
		std::cout << "Invalid tool selected: " << selectedTool << std::endl;
		break;
	}

	//std::cout << "Left mouse released" << std::endl;
}

void Application::OnMouseButtonRightReleased() {

	//if (isMouseOnGui)	// If mouse is on GUI overlay, ignore any inputs
	//	return;

	switch (selectedTool) {

	case TOOL_SELECT:
		selectToolMouseRightReleased();
		break;

	case TOOL_LINE:
		lineToolMouseRightReleased();
		break;

	case TOOL_LINE_STRIP:
		lineStripToolMouseRightReleased();
		break;

	default:
		std::cout << "Invalid tool selected: " << selectedTool << std::endl;
		break;
	}

	//std::cout << "Right mouse released" << std::endl;
}

void Application::OnMouseDragged() {

	mouseDragged();

	//std::cout << "Mouse dragged" << std::endl;
}

void Application::OnMouseHovered() {

	mouseHovered();

	//std::cout << "Mouse hovered" << std::endl;
}

void Application::OnMouseMoved() {

	mouseMoved();

	//std::cout << "Mouse hovered" << std::endl;
}

void Application::OnMouseScrolled(float amount) {

	float scroll = amount * scrollFactor;
	float factor = 1 + std::abs(scroll);

	if (scroll > 0) {
		scale *= factor;
	}
	else {
		scale /= factor;
	}

	glm::vec2 mouseToCenter = glm::vec2(panOffset.x - mouse.x + width / 2.f, panOffset.y - mouse.y + height / 2.f);

	if (scroll > 0)
		panOffset += mouseToCenter * factor - mouseToCenter;
	else
		panOffset -= mouseToCenter - mouseToCenter / factor;
}



void Application::OnLayerSelect() {

	cancelShape();

	//std::cout << "OnLayerSelect" << std::endl;
}

void Application::OnMouseEnteredLayerWindow() {

	//std::cout << "OnMouseEnteredLayerWindow" << std::endl;
}

