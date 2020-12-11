
#include "pch.h"
#include "LayerList.h"
#include "Application.h"


LayerList::LayerList() : dummyLayer("dummy", -1) {

}




void LayerList::addLayerFront(const std::string& name) {

	//std::cout << "addLayerFront" << std::endl;

	layers.push_back(Layer(name, nextLayerID));
	layerOrder.insert(layerOrder.begin(), nextLayerID);

	selectLayer(nextLayerID); // Select just created layer

	nextLayerID++;
}

void LayerList::addLayerBack(const std::string& name) {

	//std::cout << "addLayerBack" << std::endl;

	layers.push_back(Layer(name, nextLayerID));
	layerOrder.push_back(nextLayerID);

	selectLayer(nextLayerID); // Select just created layer

	nextLayerID++;
}






bool LayerList::moveLayerFront(LayerID id) {

	//std::cout << "moveLayerFront" << std::endl;
	
	// Find layerOrder index with this id
	size_t orderIndex = __findLayerOrderIndex(id);

	if (orderIndex == -1 || orderIndex < 1)
		return false;

	std::iter_swap(layerOrder.begin() + orderIndex, layerOrder.begin() + orderIndex - 1);

	return true;
}

bool LayerList::moveLayerBack(LayerID id) {

	//std::cout << "moveLayerBack" << std::endl;

	// Find layerOrder index with this id
	size_t orderIndex = __findLayerOrderIndex(id);

	if (orderIndex == -1 || orderIndex >= layerOrder.size() - 1)
		return false;

	std::iter_swap(layerOrder.begin() + orderIndex, layerOrder.begin() + orderIndex + 1);

	return true;
}





bool LayerList::deleteLayer(LayerID id) {

	//std::cout << "deleteLayer" << std::endl;

	// Find the array index with this id
	size_t arrayIndex = __findLayerArrayIndex(id);
	size_t orderIndex = __findLayerOrderIndex(id);

	if (arrayIndex == -1 || orderIndex == -1)
		return false;

	layers.erase(layers.begin() + arrayIndex);
	layerOrder.erase(layerOrder.begin() + orderIndex);

	if (selectedLayer == id) {	// If selected layer deleted, select next one
		selectedLayer = -1;
		std::cout << "WARNING: Layer was deleted, not handled correctly yet!!!" << std::endl;
	}

	return true;
}





bool LayerList::layerExists(LayerID id) {

	if (id == -1)
		return false;

	for (Layer& layer : layers) {
		if (layer.layerID == id) {
			return true;
		}
	}

	return false;
}

bool LayerList::selectLayer(LayerID id) {

	//std::cout << "selectLayer" << std::endl;

	if (!layerExists(id))
		return false;

	selectedLayer = id;
	return true;
}

LayerID LayerList::getSelectedLayerID() {
	return selectedLayer;
}

Layer* LayerList::getSelectedLayer() {
	return findLayer(selectedLayer);
}






Layer* LayerList::findLayer(LayerID id) {

	for (Layer& layer : layers) {
		if (layer.layerID == id) {
			return &layer;
		}
	}

	return nullptr;
}







std::vector<Layer>& LayerList::getLayers() {
	return layers;
}

std::vector<LayerID> LayerList::getSortedLayerIDs() {

	std::vector<LayerID> sorted;

	for (LayerID id : layerOrder) {
		sorted.push_back(id);
	}

	return sorted;
}

std::vector<LayerID> LayerList::getSortedLayerIDsReverse() {

	std::vector<LayerID> sorted;

	for (LayerID id : layerOrder) {
		sorted.insert(sorted.begin(), id);
	}

	return sorted;
}

std::vector<Layer>* LayerList::getLayerPointer() {
	return &layers;
}

std::vector<Layer*> LayerList::getLayerPointers() {

	std::vector<Layer*> pointers;

	for (size_t i = 0; i < layers.size(); i++) {
		pointers.push_back(&layers[i]);
	}

	return pointers;
}








nlohmann::json LayerList::getJson() {

	// Convert all information to a json object

	nlohmann::json jsonLayers = nlohmann::json::array();
	for (Layer& layer : layers) {
		jsonLayers.push_back(layer.getJson());
	}

	nlohmann::json jsonLayerOrder = nlohmann::json::array();
	for (LayerID id : layerOrder) {
		jsonLayerOrder.push_back(id);
	}

	nlohmann::json json = nlohmann::json({
		{ "layers", jsonLayers },
		{ "layer_order", jsonLayerOrder },
		{ "next_id", nextLayerID }
	});

	return json;
}






// Private functions

size_t LayerList::__findLayerArrayIndex(LayerID id) {

	for (size_t i = 0; i < layers.size(); i++) {
		if (layers[i].layerID == id) {
			return i;
		}
	}

	return -1;
}

size_t LayerList::__findLayerOrderIndex(LayerID id) {

	for (size_t i = 0; i < layerOrder.size(); i++) {
		if (layerOrder[i] == id) {
			return i;
		}
	}

	return -1;
}
