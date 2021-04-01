#pragma once

#include "config.h"
#include "Layer.h"

/// <summary>
/// This class guarantees, that always at least 1 layer exists
/// </summary>
class SketchFile {

	std::vector<Layer> layers;
	size_t nextLayerNameID = 1;
	size_t activeLayer = -1;	// This is the std::vector index!!!

	//bool regeneratePreviewImages = true;
	bool fileChanged = false;

	std::string filename = "";
	std::string fileLocation = "";

public:
	SketchFile() {
		filename = DEFAULT_FILENAME;
		PushLayer();
		p_CorrectLayers();
	}

	~SketchFile() {
		layers.clear();
	}

	Layer& GetActiveLayer() {
		p_CorrectLayers();
		return layers[activeLayer];
	}

	std::vector<Layer>& GetLayers() {
		p_CorrectLayers();
		return layers;
	}

	const std::vector<std::unique_ptr<GenericShape>>& GetActiveLayerShapes() {
		return GetActiveLayer().GetShapes();
	}

	void PushLayer() {
		PushLayer("Layer #" + std::to_string(nextLayerNameID++));
	}

	void PushLayer(const std::string& name) {
		layers.push_back(Layer(name));
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
		for (auto& layer : GetLayers()) {
			layer.GeneratePreview();
		}
	}

	void FileChanged() {
		fileChanged = true;
	}

	//void CreateNewFile();
	//bool LoadFile(const std::string& path, const std::string& displayName);
	//nlohmann::json GetJson();

private:
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
