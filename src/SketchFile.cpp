
#include "pch.h"
#include "SketchFile.h"

/*
SketchFile::SketchFile() {
	CreateNewFile();
}






Layer* SketchFile::GetActiveLayer() {
	Layer* layer = layers.GetActiveLayer();

	// Checking if layer is valid
	if (layer == nullptr) {
		LOG_WARN("Active Layer does not exists: Returning nullptr");
		return nullptr;
	}

	return layer;
}

LayerID SketchFile::GetActiveLayerID() {
	LayerID id = layers.GetActiveLayerID();

	// Checking if layer is valid
	if (!layers.LayerExists(id)) {
		LOG_WARN("Active Layer does not exists: Returning id -1");
		return -1;
	}

	return id;
}

LayerList& SketchFile::GetLayerList() {
	return layers;
}

std::vector<Layer>& SketchFile::GetLayers() {
	return layers.GetLayers();
}

// If invalid, vector is empty
std::vector<Shape>& SketchFile::GetActiveLayerShapes() {
	Layer* currentLayer = GetActiveLayer();

	if (!currentLayer) {		// On failure, return empty vector
		LOG_WARN(__FUNCTION__ "(): Active layer is invalid!");
		static std::vector<Shape> dummyVector;
		return dummyVector;
	}

	return currentLayer->GetShapes();
}





void SketchFile::AddNewLayer() {

	size_t nextLayerName = 1;
	for (Layer& layer : layers.GetLayers()) {
		if (layer.layerID.Get() > nextLayerName) {
			nextLayerName = layer.layerID.Get();
		}
	}

	AddNewLayer("Layer #" + std::to_string(nextLayerName));
}

void SketchFile::AddNewLayer(const std::string& name) {
	layers.AddLayerFront(name);
	regeneratePreviewImages = true;		// Set flag to regenerate previews
	FileChanged();
}

void SketchFile::DeleteLayer(LayerID id) {
	layers.DeleteLayer(id);
	regeneratePreviewImages = true;		// Set flag to regenerate previews
	FileChanged();
}



void SketchFile::MoveLayerFront(LayerID id) {
	layers.MoveLayerFront(id);
	regeneratePreviewImages = true;		// Set flag to regenerate previews
	FileChanged();
}

void SketchFile::MoveLayerBack(LayerID id) {
	layers.MoveLayerBack(id);
	regeneratePreviewImages = true;		// Set flag to regenerate previews
	FileChanged();
}



void SketchFile::SetLayerPreview(LayerID id, const Battery::Texture2D& previewImage) {
	Layer* layer = layers.FindLayer(id);
	layer->SetPreviewImage(previewImage);
}



void SketchFile::AddShape(enum ShapeType type, glm::vec2 p1, glm::vec2 p2, float lineThickness, const glm::vec4& color) {
	Layer* layer = GetActiveLayer();

	if (layer) {
		layer->AddShape(type, p1, p2, lineThickness, color);
	}
}

void SketchFile::FileChanged() {
	if (!fileChanged) {
		fileChanged = true;
		fileChangedEventFlag = true;
	}
}

void SketchFile::CreateNewFile() {

	layers.Clear();
	regeneratePreviewImages = true;
	filename = DEFAULT_FILENAME;
	fileLocation = "";
	knownLocation = false;

	AddNewLayer();

	fileChanged = false;
	fileChangedEventFlag = false;
}

bool SketchFile::LoadFile(const std::string& path, const std::string& displayName) {
	using namespace Battery;

	// Temporary containers
	LayerList tempLayers;

	// Parse the file content
	try {
		auto file = FileUtils::ReadFile(path);

		if (file.fail()) {
			LOG_ERROR("Failed to load file '" + path + "'");
			return false;
		}

		nlohmann::json data = nlohmann::json::parse(file.content());

		if (!tempLayers.LoadJson(data)) {
			LOG_ERROR("Failed to parse json file '" + path + "'");
			return false;
		}
	}
	catch (...) {
		LOG_ERROR("Failed to parse json file '" + path + "'");
		return false;
	}

	// File was successfully parsed, load to memory now
	layers.Clear();
	layers = tempLayers;
	regeneratePreviewImages = true;
	filename = displayName;

	fileChanged = false;
	fileChangedEventFlag = false;
	fileLocation = path;
	knownLocation = true;

	return true;
}

nlohmann::json SketchFile::GetJson() {
	return layers.GetJson();
}*/