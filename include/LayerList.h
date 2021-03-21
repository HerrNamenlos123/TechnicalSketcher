#pragma once

#include "pch.h"
#include "Layer.h"

class LayerList {

	std::vector<Layer> layers;
	LayerID activeLayer;

public:
	LayerList();

	void AddLayerFront(const std::string& name);
	void AddLayerBack(const std::string& name);

	bool MoveLayerFront(LayerID id);
	bool MoveLayerBack(LayerID id);

	void Clear();
	bool DeleteLayer(LayerID id);

	bool LayerExists(LayerID id);
	bool SelectLayer(LayerID id);
	LayerID GetActiveLayerID();
	Layer* GetActiveLayer();

	Layer* FindLayer(LayerID id);

	std::vector<Layer>& GetLayers();

	bool LoadJson(const nlohmann::json& json);
	nlohmann::json GetJson();


private:
	int32_t __findLayer(LayerID id);
};