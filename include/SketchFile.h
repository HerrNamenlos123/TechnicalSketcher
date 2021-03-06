#pragma once

#include "LayerList.h"
#include "config.h"

class SketchFile {

	LayerList layers;

	bool regeneratePreviewImages = true;
	bool fileChanged = false;
	bool fileChangedEventFlag = false;
	bool knownLocation = false;

public:
	std::string filename = DEFAULT_FILENAME;
	std::string fileLocation = "";

	SketchFile();

	Layer* GetActiveLayer();
	LayerID GetActiveLayerID();
	LayerList& GetLayerList();
	std::vector<Layer*> GetLayers();
	std::vector<Layer*> GetLayersReverse();
	std::vector<Shape*> GetActiveLayerShapes();

	void AddNewLayer();
	void AddNewLayer(const std::string& name);
	void DeleteLayer(LayerID id);

	void MoveLayerFront(LayerID id);
	void MoveLayerBack(LayerID id);

	void SetLayerPreview(LayerID id, const Battery::Texture2D& previewImage);

	void AddShape(enum ShapeType type, glm::vec2 p1, glm::vec2 p2, float lineThickness);
	void FileChanged();

	void CreateNewFile();
	bool LoadFile(const std::string& path, const std::string& displayName);
	nlohmann::json GetJson();
};
