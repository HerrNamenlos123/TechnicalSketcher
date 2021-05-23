#pragma once

#include "pch.h"
#include "Layer.h"

class FileContent {

	std::vector<Layer> layers;
	size_t activeLayer = -1;	// This is the std::vector index!!!

public:
	FileContent(bool init = true) {
		if (init) {
			PushLayer();
			p_CorrectLayers();
		}
	}

	Layer& GetActiveLayer() {
		p_CorrectLayers();
		return layers[activeLayer];
	}

	std::vector<Layer>& GetLayers() {
		p_CorrectLayers();
		return layers;
	}

	const std::vector<ShapePTR>& GetActiveLayerShapes() {
		return GetActiveLayer().GetShapes();
	}

	void PushLayer() {
		// Use a new name that does not exist yet
		std::string name = "Layer #1";
		size_t id = 1;

		// Increase number until it's unique
		bool repeat = false;
		do {
			repeat = false;

			for (Layer& layer : layers) {
				if (layer.name == name) {	// Name already exists
					repeat = true;

					// Increase the name number
					id++;
					name = "Layer #" + std::to_string(id);

					break;
				}
			}
		} while (repeat);

		// Now push it
		PushLayer(name);
	}

	void PushLayer(const std::string& name) {
		layers.push_back(Layer(name));
		ActivateLayer(layers[layers.size() - 1].GetID()); // Select just created layer
		GeneratePreviews();
	}

	void PushLayer(Layer&& layer) {
		layers.push_back(layer);
		ActivateLayer(layers[layers.size() - 1].GetID()); // Select just created layer
		GeneratePreviews();
	}

	bool ActivateLayer(LayerID id) {

		for (size_t i = 0; i < layers.size(); i++) {
			if (layers[i].GetID() == id) {
				activeLayer = i;
				return true;
			}
		}

		LOG_WARN(__FUNCTION__"(): Can't activate Layer #{}: Does not exist!", id);
		return false;
	}

	bool RemoveLayer(LayerID id) {

		// Find layer and remove it
		for (size_t i = 0; i < layers.size(); i++) {
			if (layers[i].GetID() == id) {
				layers.erase(layers.begin() + i);
				p_CorrectLayers();
				return true;
			}
		}

		return false;
	}

	std::optional<std::reference_wrapper<Layer>> FindLayer(LayerID id) {
		for (Layer& layer : layers) {
			if (layer.GetID() == id) {
				return std::make_optional<std::reference_wrapper<Layer>>(layer);
			}
		}

		return std::nullopt;
	}

	bool MoveLayerFront(LayerID id) {
		p_CorrectLayers();

		size_t index = p_FindLayerIndex(id);
		if (index == -1 || index < 1) {
			return false;
		}

		LOG_TRACE("Moving Layer '{}' with id #{} front", layers[index].name, layers[index].GetID());
		std::iter_swap(layers.begin() + index, layers.begin() + index - 1);
		return true;
	}

	bool MoveLayerBack(LayerID id) {

		size_t index = p_FindLayerIndex(id);
		if (index == -1 || index >= layers.size() - 1) {
			return false;
		}

		LOG_TRACE("Moving Layer '{}' with id #{} back", layers[index].name, layers[index].GetID());
		std::iter_swap(layers.begin() + index, layers.begin() + index + 1);
		return true;
	}

	void GeneratePreviews() {
		for (auto& layer : layers) {
			layer.GeneratePreview();
		}
	}

public:
	// Correct activeLayer, if it's -1 or too large
	void p_CorrectLayers() {

		if (layers.size() == 0) {
			PushLayer();
		}

		if (activeLayer == -1) {
			activeLayer = 0;
		}
		else if (activeLayer >= layers.size()) {
			activeLayer = layers.size() - 1;
		}
	}

	// Find the std::vector index of the layer with the specified id: -1 if not found
	size_t p_FindLayerIndex(ShapeID id) {

		for (size_t i = 0; i < layers.size(); i++) {
			if (layers[i].GetID() == id) {
				return i;
			}
		}

		return -1;
	}

	bool p_IsIndexValid(size_t index) {
		return index < layers.size();
	}
};
