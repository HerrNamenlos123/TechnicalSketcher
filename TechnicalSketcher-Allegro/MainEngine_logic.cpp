
#include "pch.h"
#include "MainEngine.h"
#include "utils.h"


void MainEngine::generateLayerPreviews() {
	timestamp_t start = micros();

	for (Layer& layer : layers.layers) {
		ALLEGRO_BITMAP* bitmap = createLayerPreviewBitmap(layer.layerID, gui_hoverWindowSize.x, gui_hoverWindowSize.y);
		layer.cloneFrom(bitmap);
		al_destroy_bitmap(bitmap);
	}
	gui_previewChanged = false;
	std::cout << "Generated preview bitmaps, took " << (micros() - start) / 1000.0 << " ms" << std::endl;
}

void MainEngine::addLayer() {
	addLayer("Layer #" + std::to_string(maxLayers));
}

void MainEngine::addLayer(const std::string& name) {

	cancelShape();

	layers.addLayerFront(name);
	maxLayers++;

	gui_previewChanged = true;
}

void MainEngine::addLine(glm::vec2 p1, glm::vec2 p2) {
	if (p1 != p2) {
		layers.findLayer(layers.selectedLayer).addShape(SHAPE_LINE, p1, p2, ctrl_currentLineThickness);
	}

	gui_previewChanged = true;
}

void MainEngine::changeMode(int mode) {

	cancelShape();
	selectedShapes.clear();

	switch (mode) {

	case MODE_SELECT:
		cursorMode = MODE_SELECT;
		showPreviewPoint = false;
		draggingSelectionBox = false;
		break;

	case MODE_LINE:
		cursorMode = MODE_LINE;
		previewPoint = mouseSnapped;
		showPreviewPoint = true;
		draggingLine = false;
		draggingSelectionBox = false;
		break;

	case MODE_LINE_STRIP:
		cursorMode = MODE_LINE_STRIP;
		previewPoint = mouseSnapped;
		showPreviewPoint = true;
		draggingLine = false;
		draggingSelectionBox = false;
		break;

	default:
		break;
	}
}

bool MainEngine::isShapeSelected(ShapeID shape) {

	for (size_t i = 0; i < selectedShapes.size(); i++) {
		if (selectedShapes[i] == shape) {
			return true;
		}
	}

	return false;
}

bool MainEngine::deleteShape(ShapeID shape) {
	gui_previewChanged = true;

	return layers.findLayer(layers.selectedLayer).removeShape(shape);
}

void MainEngine::cancelShape() {
	showPreviewPoint = true;
	draggingLine = false;
}
