#pragma once

#include "pch.h"
#include "Layer.h"

class LayerList {

	Layer dummyLayer;			// empty dummy layer for references to point to

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

	bool deleteLayer(LayerID id);

	bool layerExists(LayerID id);
	bool selectLayer(LayerID id);
	LayerID getSelectedLayerID();
	Layer& getSelectedLayer();

	Layer& findLayer(LayerID id);
	std::vector<Layer>& getLayers();
	std::vector<LayerID> getSortedLayerIDs();
	std::vector<LayerID> getSortedLayerIDsReverse();

	nlohmann::json getJson();


private:
	size_t __findLayerArrayIndex(LayerID id);
	size_t __findLayerOrderIndex(LayerID id);
};