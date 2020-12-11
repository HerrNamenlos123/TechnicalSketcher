
#include "pch.h"
#include "SketchFile.h"


SketchFile::SketchFile() {
	nextLayerName = 0;

	addNewLayer();	// Add first layer
}






Layer* SketchFile::getCurrentLayer() {
	
	// Checking if layer is valid
	if (!layers.layerExists(layers.getSelectedLayerID()))
		return nullptr;

	return layers.getSelectedLayer();
}

LayerList& SketchFile::getLayerList() {
	return layers;
}

std::vector<Layer*> SketchFile::getLayers() {
	return layers.getLayerPointers();
}

// If invalid, vector is empty
std::vector<Shape*> SketchFile::getCurrentLayerShapes() {

	Layer* currentLayer = getCurrentLayer();

	if (!currentLayer)		// On failure, return empty vector
		return std::vector<Shape*>();

	return currentLayer->getShapes();
}





void SketchFile::addNewLayer() {
	addNewLayer("Layer #" + std::to_string(nextLayerName));
	setPreviewRegenerateFlag();
}

void SketchFile::addNewLayer(const std::string& name) {
	layers.addLayerFront(name);
	nextLayerName++;
}

void SketchFile::deleteLayer(LayerID id) {
	layers.deleteLayer(id);
}



void SketchFile::moveLayerFront(LayerID id) {
	layers.moveLayerFront(id);
}

void SketchFile::moveLayerBack(LayerID id) {
	layers.moveLayerBack(id);
}



void SketchFile::setLayerPreview(LayerID id, ALLEGRO_BITMAP* bitmap) {
	Layer* layer = layers.findLayer(id);
	layer->cloneFrom(bitmap);
}

void SketchFile::setPreviewRegenerateFlag() {
	regeneratePreviews = true;
}

void SketchFile::clearPreviewRegenerateFlag() {
	regeneratePreviews = false;
}

bool SketchFile::getPreviewRegenerateFlag() {
	return regeneratePreviews;
}



void SketchFile::addShape(enum ShapeType type, glm::vec2 p1, glm::vec2 p2, float lineThickness) {
	Layer* layer = getCurrentLayer();

	if (layer) {
		layer->addShape(type, p1, p2, lineThickness);
	}
}

nlohmann::json SketchFile::getJson() {

	nlohmann::json json;

	json = nlohmann::json{ { "next_layer_id", nextLayerName }, layers.getJson() };

	return json;
}