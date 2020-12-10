
#include "pch.h"
#include "Application.h"
#include "config.h"
#include "fonts.h"


// Main engine functions are here, others are in other files like Application_GUI.cpp or Application_Events
// View Application.h to see where all definitions are

Application::Application() {
}

void Application::setup() {

	loadImGuiFonts();

	changeMode(TOOL_SELECT);
	addLayer();

	generateLayerPreviews();
	showPreviewPoint = false;
}

void Application::draw() {

	// Convert mouse position from window coordinates to workspace coordinates
	// and the snapped version in the grid
	updateMousePositions();

	// Do some stuff like processing the preview flag
	prepareGUI();

	// Update all ImGui window overlays
	ribbonWindow.update({ width, height });
	layerWindow.update(layers);
	toolboxWindow.update();
	mouseInfoWindow.update({ width, height }, mouse_workspace, mouseSnapped_workspace);

	// Handle and call all application events and callbacks
	handleEvents();

	// Now render various elements on the screen
	renderApplication();

	//std::cout << "ShapeMovingMode: " << shapeMovingMode << std::endl;
	//std::cout << "HoveredShape: " << hoveredShape << std::endl;
	//std::cout << "MouseOnShape: " << mouseOnShape << std::endl;
	//std::cout << "SelectedShape: #" << layers.getSelectedLayerID() << std::endl;
}

void Application::destroy() {
}

void Application::keyPressed(int keycode, int unicode, unsigned int modifiers, bool repeat) {
	
	if (keycode == ALLEGRO_KEY_ESCAPE) {
		close();
		return;
	}

	std::vector<Shape>& shapes = layers.getSelectedLayer().getShapes();

	if (keycode == ALLEGRO_KEY_DELETE) {

		// Delete all selected shapes
		for (size_t i = 0; i < selectedShapes.size(); i++) {
			deleteShape(selectedShapes[i]);
		}
		selectedShapes.clear();
		previewRegenerateFlag = true;
	}
	else if (keycode == ALLEGRO_KEY_UP) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape& shape : shapes) {
				if (shape.shapeID == selectedShapes[i]) {
					shape.p1.y -= snapSize;
					shape.p2.y -= snapSize;
					break;
				}
			}
		}
		previewRegenerateFlag = true;

	}
	else if (keycode == ALLEGRO_KEY_DOWN) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape& shape : shapes) {
				if (shape.shapeID == selectedShapes[i]) {
					shape.p1.y += snapSize;
					shape.p2.y += snapSize;
					break;
				}
			}
		}
		previewRegenerateFlag = true;

	}
	else if (keycode == ALLEGRO_KEY_RIGHT) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape& shape : shapes) {
				if (shape.shapeID == selectedShapes[i]) {
					shape.p1.x += snapSize;
					shape.p2.x += snapSize;
					break;
				}
			}
		}
		previewRegenerateFlag = true;

	}
	else if (keycode == ALLEGRO_KEY_LEFT) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape& shape : shapes) {
				if (shape.shapeID == selectedShapes[i]) {
					shape.p1.x -= snapSize;
					shape.p2.x -= snapSize;
					break;
				}
			}
		}
		previewRegenerateFlag = true;
	}

	if (keycode == ALLEGRO_KEY_S && (modifiers & ALLEGRO_KEYMOD_CTRL)) {
		saveFile("test.tsk");
	}
}

void Application::mouseScrolled(int x, int y) {

	OnMouseScrolled(x + y);
}
