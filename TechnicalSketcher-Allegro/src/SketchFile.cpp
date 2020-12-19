
#include "pch.h"
#include "SketchFile.h"


SketchFile::SketchFile() {
	openNewFile();
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
}

void SketchFile::addNewLayer(const std::string& name) {
	layers.addLayerFront(name);
	nextLayerName++;
	setPreviewRegenerateFlag();
	fileChanged();
}

void SketchFile::deleteLayer(LayerID id) {
	layers.deleteLayer(id);
	setPreviewRegenerateFlag();
	fileChanged();
}



void SketchFile::moveLayerFront(LayerID id) {
	layers.moveLayerFront(id);
	setPreviewRegenerateFlag();
	fileChanged();
}

void SketchFile::moveLayerBack(LayerID id) {
	layers.moveLayerBack(id);
	setPreviewRegenerateFlag();
	fileChanged();
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

void SketchFile::fileChanged() {
	if (!__fileChanged) {
		__fileChanged = true;
		__fileChangedEventFlag = true;
	}
}

void SketchFile::openNewFile() {

	layers.clear();
	nextLayerName = 0;
	regeneratePreviews = true;
	filename = DEFAULT_FILENAME;

	addNewLayer();

	__fileChanged = false;
	__fileChangedEventFlag = false;
	__fileLocation = "";
}

bool SketchFile::loadFile(const std::string& content, const std::string& path, const std::string& displayName) {

	// Temporary containers
	LayerList tempLayers;
	size_t tempNextName;
	
	// Parse the file content
	try {
		nlohmann::json data = nlohmann::json::parse(content);

		tempNextName = data["next_layer_name"];

		if (!tempLayers.loadJson(data)) {
			return false;
		}
	}
	catch (...) {
		return false;
	}

	// File was successfully parsed, load to memory now
	layers.clear();
	layers = tempLayers;
	nextLayerName = tempNextName;
	regeneratePreviews = true;
	filename = displayName;

	__fileChanged = false;
	__fileChangedEventFlag = false;
	__fileLocation = path;

	return true;
}

nlohmann::json SketchFile::getJson() {

	nlohmann::json json;

	json = layers.getJson();
	json["next_layer_name"] = nextLayerName;

	return json;
}