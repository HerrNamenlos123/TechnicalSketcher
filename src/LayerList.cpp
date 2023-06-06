//
//#include "pch.hpp"
//#include "LayerList.h"

/*


void LayerList::AddLayerFront(const std::string& name) {
	Layer layer(name);
	LayerID id = layer.GetID();

	b::log::trace("Added Layer " + name + " to front with id " + std::to_string(id));
	layers.insert(layers.begin(), std::move(layer));
}

void LayerList::AddLayerBack(const std::string& name) {
	Layer layer(name);
	LayerID id = layer.GetID();

	b::log::trace("Added Layer " + name + " to back with id " + std::to_string(id.Get()));
	layers.push_back(std::move(layer));
	SelectLayer(id); // Select just created layer
}




bool LayerList::LoadJson(const nlohmann::json& json) {

	// Try to parse data
	try {

		layers.clear();
		for (size_t i = 0; i < order.size(); i++) {
			layers.push_back(static_cast<LayerID>(order[i]));
		}

		for (nlohmann::json j : json["layers"]) {
			layers.push_back(Layer(j));
		}

		selectedLayer = layerOrder[0];
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
*/