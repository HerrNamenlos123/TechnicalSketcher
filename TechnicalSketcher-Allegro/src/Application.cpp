
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

	generateLayerPreviews();
	showPreviewPoint = false;

	updateWindowTitle();
}

void Application::draw() {

	// Convert mouse position from window coordinates to workspace coordinates
	// and the snapped version in the grid
	updateMousePositions();

	// Do some stuff like processing the preview flag
	prepareGUI();

	// Update all ImGui window overlays
	ribbonWindow.update({ width, height });
	layerWindow.update(file);
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

	if (wantsToClose) {
		if (file.__fileChanged) {

			// Ask the user if file needs to be saved
			int r = al_show_native_message_box(display,
				"Save changes",
				(file.filename + " contains unsaved changes").c_str(),
				"Do you want to save it?",
				NULL,
				ALLEGRO_MESSAGEBOX_YES_NO | ALLEGRO_MESSAGEBOX_WARN
			);

			if (r == 1) {	// Yes clicked
				if (saveFile()) {
					std::cout << "Main loop set close flag" << std::endl;
					close();
				}
				else {
					// Reset flag and continue program
					wantsToClose = false;
				}
			}
			else {
				std::cout << "Main loop set close flag" << std::endl;
				close();
			}
		}
		else {
			std::cout << "Main loop set close flag" << std::endl;
			close();
		}
	}
}

void Application::destroy() {
}

void Application::keyPressed(int keycode, int unicode, unsigned int modifiers, bool repeat) {

	// Handle key events

	if (keycode == ALLEGRO_KEY_ESCAPE) {
		wantsToClose = true;
	}

	// Ctrl + S
	if (keycode == ALLEGRO_KEY_S && (modifiers & ALLEGRO_KEYMOD_CTRL)) {
		if (getKey(ALLEGRO_KEYMOD_SHIFT)) {
			saveFile(true);	// Save as
		}
		else {
			saveFile();		// Save normally
		}
	}

	// Ctrl + O
	if (keycode == ALLEGRO_KEY_O && (modifiers & ALLEGRO_KEYMOD_CTRL)) {
		//openFile();
	}

	Layer* layer = file.getCurrentLayer();

	if (keycode == ALLEGRO_KEY_DELETE) {

		// Delete all selected shapes
		for (size_t i = 0; i < selectedShapes.size(); i++) {
			deleteShape(selectedShapes[i]);
		}
		selectedShapes.clear();
		file.setPreviewRegenerateFlag();
		file.fileChanged();
	}
	else if (keycode == ALLEGRO_KEY_UP) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape* shape : layer->getShapes()) {
				if (shape->shapeID == selectedShapes[i]) {
					shape->p1.y -= snapSize;
					shape->p2.y -= snapSize;
					break;
				}
			}
		}
		file.setPreviewRegenerateFlag();
		file.fileChanged();

	}
	else if (keycode == ALLEGRO_KEY_DOWN) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape* shape : layer->getShapes()) {
				if (shape->shapeID == selectedShapes[i]) {
					shape->p1.y += snapSize;
					shape->p2.y += snapSize;
					break;
				}
			}
		}
		file.setPreviewRegenerateFlag();
		file.fileChanged();

	}
	else if (keycode == ALLEGRO_KEY_RIGHT) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape* shape : layer->getShapes()) {
				if (shape->shapeID == selectedShapes[i]) {
					shape->p1.x += snapSize;
					shape->p2.x += snapSize;
					break;
				}
			}
		}
		file.setPreviewRegenerateFlag();
		file.fileChanged();

	}
	else if (keycode == ALLEGRO_KEY_LEFT) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape* shape : layer->getShapes()) {
				if (shape->shapeID == selectedShapes[i]) {
					shape->p1.x -= snapSize;
					shape->p2.x -= snapSize;
					break;
				}
			}
		}
		file.setPreviewRegenerateFlag();
		file.fileChanged();
	}
}

void Application::mouseScrolled(int x, int y) {

	OnMouseScrolled(x + y);
}
