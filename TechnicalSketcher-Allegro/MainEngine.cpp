
#include "pch.h"
#include "MainEngine.h"

// Main engine functions are here, others are in other files like MainEngine_GUI.cpp or MainEngine_Events

MainEngine::MainEngine() : layers(this) {
}

void MainEngine::setup() {

	changeMode(MODE_SELECT);
	addLayer();
	layers.addLayerFront("test1");
	layers.addLayerFront("test2");
	layers.addLayerFront("test3");
	layers.addLayerFront("test2");
	layers.addLayerFront("test1");

	loadFonts();

	generateLayerPreviews();
	showPreviewPoint = false;
}

void MainEngine::draw() {

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoNav;
	//window_flags |= ImGuiWindowFlags_NoBackground;
	//window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::PushFont(font_pt20);

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(width, gui_leftUpperViewportCorner.y));
	bool isOpen = true;
	ImGui::GetStyle().WindowRounding = 0.0f;

	ImGui::Begin("TopBar", &isOpen, window_flags);
	ImGui::End();

	ImGui::ShowDemoWindow();
	drawLayersWindow();
	drawToolboxWindow();
	drawMousePositionWindow();

	ImGui::PopFont();

	handleEvents();



	// Now everything is calculated, display it now

	background(255);

	if (draggingSelectionBox) {	// Fill should be in the back
		filledRectangle(previewPoint, mouse, gfx_selectionBoxFillColor);
	}

	drawGrid();
	drawLayersOnScreen();

	if (draggingLine) {
		drawLine(previewLineStart, mouseSnapped, ctrl_currentLineThickness, color(0));
	}
	if (showPreviewPoint) {
		drawPreviewPoint(previewPoint);
	}
	if (draggingSelectionBox) {	// Outline in the front
		outlinedRectangle(previewPoint, mouse, gfx_selectionBoxColor, 1);
	}
}

void MainEngine::destroy() {
}

void MainEngine::keyPressed(int keycode, unsigned int modifiers) {
	if (keycode == ALLEGRO_KEY_ESCAPE) {
		close();
	}

	std::vector<Shape>& shapes = layers.findLayer(layers.selectedLayer).getShapes();

	if (keycode == ALLEGRO_KEY_DELETE) {

		// Delete all selected shapes
		for (size_t i = 0; i < selectedShapes.size(); i++) {
			deleteShape(selectedShapes[i]);
		}
		selectedShapes.clear();
	}
	else if (keycode == ALLEGRO_KEY_UP) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape& shape : shapes) {
				if (shape.shapeID == selectedShapes[i]) {
					shape.p1.y -= ctrl_snapTo;
					shape.p2.y -= ctrl_snapTo;
					break;
				}
			}
		}

	}
	else if (keycode == ALLEGRO_KEY_DOWN) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape& shape : shapes) {
				if (shape.shapeID == selectedShapes[i]) {
					shape.p1.y += ctrl_snapTo;
					shape.p2.y += ctrl_snapTo;
					break;
				}
			}
		}

	}
	else if (keycode == ALLEGRO_KEY_RIGHT) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape& shape : shapes) {
				if (shape.shapeID == selectedShapes[i]) {
					shape.p1.x += ctrl_snapTo;
					shape.p2.x += ctrl_snapTo;
					break;
				}
			}
		}

	}
	else if (keycode == ALLEGRO_KEY_LEFT) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			for (Shape& shape : shapes) {
				if (shape.shapeID == selectedShapes[i]) {
					shape.p1.x -= ctrl_snapTo;
					shape.p2.x -= ctrl_snapTo;
					break;
				}
			}
		}

	}
}

void MainEngine::mouseScrolled(int x, int y) {

	if (gui_mouseOnGui)
		return;

	float scroll = (x + y) * scrollFactor;
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
