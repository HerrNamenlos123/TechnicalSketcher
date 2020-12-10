
#include "pch.h"
#include "LayerList.h"
#include "Application.h"


LayerList::LayerList() : dummyLayer("dummy", -1) {

}




void LayerList::addLayerFront(const std::string& name) {

	std::cout << "addLayerFront" << std::endl;

	layers.push_back(Layer(name, nextLayerID));

	// Select created layer if none is selected yet
	if (selectedLayer == -1) {
		selectedLayer = nextLayerID;
	}

	layerOrder.insert(layerOrder.begin(), nextLayerID);

	nextLayerID++;
}

void LayerList::addLayerBack(const std::string& name) {

	std::cout << "addLayerBack" << std::endl;

	layers.push_back(Layer(name, nextLayerID));

	if (selectedLayer == -1) {
		selectedLayer = nextLayerID;
	}

	layerOrder.push_back(nextLayerID);

	nextLayerID++;
}






bool LayerList::moveLayerFront(LayerID id) {

	std::cout << "moveLayerFront" << std::endl;
	
	// Find layerOrder index with this id
	size_t orderIndex = __findLayerOrderIndex(id);

	if (orderIndex == -1 || orderIndex < 1)
		return false;

	std::iter_swap(layerOrder.begin() + orderIndex, layerOrder.begin() + orderIndex - 1);

	return true;
}

bool LayerList::moveLayerBack(LayerID id) {

	std::cout << "moveLayerBack" << std::endl;

	// Find layerOrder index with this id
	size_t orderIndex = __findLayerOrderIndex(id);

	if (orderIndex == -1 || orderIndex >= layerOrder.size() - 1)
		return false;

	std::iter_swap(layerOrder.begin() + orderIndex, layerOrder.begin() + orderIndex + 1);

	return true;
}





bool LayerList::deleteLayer(LayerID id) {

	std::cout << "deleteLayer" << std::endl;

	// Find the array index with this id
	size_t arrayIndex = __findLayerArrayIndex(id);
	size_t orderIndex = __findLayerOrderIndex(id);

	if (arrayIndex == -1 || orderIndex == -1)
		return false;

	layers.erase(layers.begin() + arrayIndex);
	layerOrder.erase(layerOrder.begin() + orderIndex);

	return true;
}





bool LayerList::layerExists(LayerID id) {

	for (Layer& layer : layers) {
		if (layer.layerID == id) {
			return true;
		}
	}

	return false;
}

bool LayerList::selectLayer(LayerID id) {

	std::cout << "selectLayer" << std::endl;

	if (!layerExists(id))
		return false;

	selectedLayer = id;
	return true;
}

LayerID LayerList::getSelectedLayerID() {
	return selectedLayer;
}

Layer& LayerList::getSelectedLayer() {
	return findLayer(selectedLayer);
}






Layer& LayerList::findLayer(LayerID id) {

	for (Layer& layer : layers) {
		if (layer.layerID == id) {
			return layer;
		}
	}

	throw std::logic_error("Can't find Layer with ID " + std::to_string(id) + " in LayerList");
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
