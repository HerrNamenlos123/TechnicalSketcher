#pragma once

#include "pch.h"
#include "Layer.h"

class LayerList {

	std::vector<Layer> layers;
	std::vector<LayerID> layerOrder;
	LayerID selectedLayer = -1;

	LayerID nextLayerID = 0;

public:

	LayerList();

	void addLayerFront(const std::string& name);
	void addLayerBack(const std::string& name);

	bool moveLayerFront(LayerID id);
	bool moveLayerBack(LayerID id);

	void clear();
	bool deleteLayer(LayerID id);

	bool layerExists(LayerID id);
	bool selectLayer(LayerID id);
	LayerID getSelectedLayerID();
	Layer* getSelectedLayer();

	Layer* findLayer(LayerID id);
	std::vector<Layer>& getLayers();
	std::vector<LayerID> getSortedLayerIDs();
	std::vector<LayerID> getSortedLayerIDsReverse();
	std::vector<Layer>* getLayerPointer();
	std::vector<Layer*> getLayerPointers();

	bool loadJson(nlohmann::json json);
	nlohmann::json getJson();


private:
	size_t __findLayerArrayIndex(LayerID id);
	size_t __findLayerOrderIndex(LayerID id);
};