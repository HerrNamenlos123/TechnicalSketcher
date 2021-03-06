
#include "pch.h"
#include "LayerList.h"


LayerList::LayerList() {

}




void LayerList::AddLayerFront(const std::string& name) {
	Layer layer(name);
	LayerID id = layer.layerID;

	LOG_TRACE("Added Layer " + name + " to front with id " + std::to_string(id.Get()));
	layers.insert(layers.begin(), std::move(layer));
	SelectLayer(id); // Select just created layer
}

void LayerList::AddLayerBack(const std::string& name) {
	Layer layer(name);
	LayerID id = layer.layerID;

	LOG_TRACE("Added Layer " + name + " to back with id " + std::to_string(id.Get()));
	layers.push_back(std::move(layer));
	SelectLayer(id); // Select just created layer
}






bool LayerList::MoveLayerFront(LayerID id) {

	size_t layerIndex = __findLayer(id);
	if (layerIndex == -1 || layerIndex < 1) {
		LOG_ERROR(__FUNCTION__ "(): Can't move Layer #" + std::to_string(id.Get()));
		return false;
	}

	LOG_TRACE("Moving Layer " + layers[layerIndex].name + " #" + std::to_string(layers[layerIndex].layerID.Get()));
	std::iter_swap(layers.begin() + layerIndex, layers.begin() + layerIndex - 1);
	return true;
}

bool LayerList::MoveLayerBack(LayerID id) {

	size_t layerIndex = __findLayer(id);
	if (layerIndex == -1 || layerIndex >= layers.size() - 1) {
		LOG_ERROR(__FUNCTION__ "(): Can't move Layer #" + std::to_string(id.Get()));
		return false;
	}

	LOG_TRACE("Moving Layer " + layers[layerIndex].name + " #" + std::to_string(layers[layerIndex].layerID.Get()));
	std::iter_swap(layers.begin() + layerIndex, layers.begin() + layerIndex + 1);
	return true;
}



void LayerList::Clear() {
	layers.clear();
	activeLayer = -1;
}





bool LayerList::DeleteLayer(LayerID id) {

	size_t layerIndex = __findLayer(id);
	if (layerIndex == -1) {
		LOG_ERROR(__FUNCTION__ "(): Can't move Layer #" + std::to_string(id.Get()));
		return false;
	}

	layers.erase(layers.begin() + layerIndex);

	if (activeLayer >= layers.size()) {
		activeLayer = -1;
		LOG_WARN(__FUNCTION__ "(): No more layer to choose!");
	}

	return true;
}





bool LayerList::LayerExists(LayerID id) {

	if (id.Get() == -1)
		return false;

	for (Layer& layer : layers) {
		if (layer.layerID == id) {
			return true;
		}
	}

	return false;
}

bool LayerList::SelectLayer(LayerID id) {

	if (!LayerExists(id)) {
		LOG_WARN(__FUNCTION__ "(): Can't choose Layer #" + std::to_string(id.Get()) + ": Does not exist!");
		return false;
	}

	activeLayer = id;
	LOG_TRACE("Chose Layer #" + std::to_string(id.Get()));
	return true;
}

LayerID LayerList::GetActiveLayerID() {
	return activeLayer;
}

Layer* LayerList::GetActiveLayer() {
	return FindLayer(activeLayer);
}






Layer* LayerList::FindLayer(LayerID id) {

	for (Layer& layer : layers) {
		if (layer.layerID == id) {
			return &layer;
		}
	}

	return nullptr;
}







std::vector<Layer*> LayerList::GetLayers() {
	std::vector<Layer*> pointers;

	for (size_t i = 0; i < layers.size(); i++) {
		pointers.push_back(&layers[i]);
	}

	return pointers;
}

std::vector<Layer*> LayerList::GetLayersReverse() {
	std::vector<Layer*> pointers;

	// Loop in reverse
	for (size_t i = layers.size() - 1; i < layers.size(); i--) {
		pointers.push_back(&layers[i]);
	}

	return pointers;
}






bool LayerList::LoadJson(const nlohmann::json& json) {

	// Try to parse data
	try {

		/*layers.clear();
		for (size_t i = 0; i < order.size(); i++) {
			layers.push_back(static_cast<LayerID>(order[i]));
		}

		for (nlohmann::json j : json["layers"]) {
			layers.push_back(Layer(j));
		}

		selectedLayer = layerOrder[0];*/
	}
	catch (...) {
		return false;
	}

	return true;
}

nlohmann::json LayerList::GetJson() {
	// Convert all information to a json object
	nlohmann::json jsonLayers = nlohmann::json::array();
	for (Layer& layer : layers) {
		jsonLayers.push_back(layer.GetJson());
	}

	nlohmann::json json = nlohmann::json({
		{ "layers", jsonLayers }
		});

	return json;
}






// Private functions

int32_t LayerList::__findLayer(LayerID id) {

	for (int32_t i = 0; i < (int32_t)layers.size(); i++) {
		if (layers[i].layerID == id) {
			return i;
		}
	}

	return -1;
}
