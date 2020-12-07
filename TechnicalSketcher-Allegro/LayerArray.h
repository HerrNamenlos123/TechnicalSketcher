#pragma once

#include "pch.h"
#include "Layer.h"

class MainEngine;	// Forward declaration of MainEngine

class LayerArray {

	Layer dummyLayer;

	LayerID nextLayerID = 0;
	MainEngine* engineReference;

public:
	std::vector<Layer> layers;
	LayerID selectedLayer = -1;

	LayerArray(MainEngine* reference);

	void addLayerFront(const std::string& name);
	void addLayerBack(const std::string& name);

	bool moveLayerFront(size_t index);
	bool moveLayerBack(size_t index);

	Layer& findLayer(LayerID id);

	void updateGUI();
};