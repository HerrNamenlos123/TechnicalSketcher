#pragma once

#include "LayerList.h"
#include "config.h"

class SketchFile {

	LayerList layers;
	size_t nextLayerName;

	bool regeneratePreviews = true;

public:
	std::string filename = DEFAULT_FILENAME;
	bool __fileChanged = false;
	bool __fileChangedEventFlag = false;
	std::string __fileLocation = "";

	SketchFile();

	Layer* getCurrentLayer();
	LayerList& getLayerList();
	std::vector<Layer*> getLayers();
	std::vector<Shape*> getCurrentLayerShapes();

	void addNewLayer();
	void addNewLayer(const std::string& name);
	void deleteLayer(LayerID id);

	void moveLayerFront(LayerID id);
	void moveLayerBack(LayerID id);

	void setLayerPreview(LayerID id, ALLEGRO_BITMAP* bitmap);
	void setPreviewRegenerateFlag();
	void clearPreviewRegenerateFlag();
	bool getPreviewRegenerateFlag();

	void addShape(enum ShapeType type, glm::vec2 p1, glm::vec2 p2, float lineThickness);
	void fileChanged();

	void openNewFile();
	bool loadFile(const std::string& content, const std::string& path, const std::string& displayName);
	nlohmann::json getJson();
};
