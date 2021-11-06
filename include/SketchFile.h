#pragma once

#include "pch.h"
#include "config.h"
#include "FileContent.h"
#include "Layer.h"

/// <summary>
/// This class guarantees, that always at least 1 layer exists
/// </summary>
class SketchFile {

	FileContent content;

	bool fileChanged = false;
	std::string filename = DEFAULT_FILENAME;	// Filename contains extension
	std::string fileLocation = "";

public:
	glm::vec4 backgroundColor = DEFAULT_BACKGROUND_COLOR;

	SketchFile() {

	}

	void PushLayer() {
		content.PushLayer();
		fileChanged = true;
	}

	void PushLayer(const std::string& name) {
		content.PushLayer(name);
		fileChanged = true;
	}

	void PushLayer(Layer&& layer) {
		content.PushLayer(std::move(layer));
		fileChanged = true;
	}

	void GeneratePreviews() {
		content.GeneratePreviews();
	}

	const std::vector<Layer>& GetLayers() {
		return content.GetLayers();
	}

	const Layer& GetActiveLayer() {
		return content.GetActiveLayer();
	}

	Layer DuplicateActiveLayer() {
		return content.GetActiveLayer().Duplicate();
	}

	bool SetLayerName(LayerID id, const char* name) {
		auto layer = content.FindLayer(id);

		if (layer.has_value()) {	// Layer was found
			layer->get().name = name;
			fileChanged = true;
			return true;
		}

		return false;
	}

	void SaveActiveLayerState() {
		content.GetActiveLayer().SaveState();
	}

	void UndoAction() {
		content.GetActiveLayer().UndoAction();
	}

	bool MoveLayerFront(LayerID id) {
		if (!content.MoveLayerFront(id)) {
			return false;
		}
		fileChanged = true;
		return true;
	}

	bool MoveLayerBack(LayerID id) {
		if (!content.MoveLayerBack(id)) {
			return false;
		}
		fileChanged = true;
		return true;
	}

	bool RemoveLayer(LayerID id) {
		if (!content.RemoveLayer(id)) {
			return false;
		}
		fileChanged = true;
		return true;
	}

	bool ActivateLayer(LayerID id) {
		return content.ActivateLayer(id);
	}

	void AddShape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness, const glm::vec4& color) {
		content.GetActiveLayer().AddShape(type, p1, p2, thickness, color);
		fileChanged = true;
	}

	void AddShape(enum class ShapeType type, glm::vec2 center, float radius, float thickness, const glm::vec4& color) {
		content.GetActiveLayer().AddShape(type, center, radius, thickness, color);
		fileChanged = true;
	}

	void AddShape(enum class ShapeType type, glm::vec2 center, float radius, float startAngle, float endAngle, float thickness, const glm::vec4& color) {
		content.GetActiveLayer().AddShape(type, center, radius, startAngle, endAngle, thickness, color);
		fileChanged = true;
	}

	void AddShapes(std::vector<ShapePTR>&& shapes) {
		content.GetActiveLayer().AddShapes(std::move(shapes));
		fileChanged = true;
	}

	bool RemoveShapes(const std::vector<ShapeID>& ids) {
		if (content.GetActiveLayer().RemoveShapes(ids)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesLeft(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesLeft(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesRight(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesRight(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesUp(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesUp(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesDown(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesDown(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapes(const std::vector<ShapeID>& ids, glm::vec2 amount) {
		if (content.GetActiveLayer().MoveShapes(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	void ShowPropertiesWindow(ShapeID id) {
		auto opt = content.GetActiveLayer().FindShape(id);

		if (opt.has_value()) {
			if (opt.value().get().ShowPropertiesWindow()) {
				fileChanged = true;
			}
		}
	}

	std::optional<std::reference_wrapper<const GenericShape>> FindShape(const ShapeID& id) {
		return content.GetActiveLayer().FindShape(id);
	}

	void UpdateWindowTitle();

	void FileChanged() {
		fileChanged = true;
	}

	nlohmann::json GetJsonFromShapes(const std::vector<ShapeID>& ids) {
		nlohmann::json json = nlohmann::json();

		for (auto id : ids) {
			auto shape = FindShape(id);

			if (shape.has_value()) {
				json.push_back(shape.value().get().GetJson());
			}
		}

		return json;
	}

	bool ContainsChanges() {
		return fileChanged;
	}

	bool SaveFile(bool saveAs = false);
	bool OpenFile();
	bool OpenEmptyFile();
	bool OpenFile(const std::string& path, bool silent = false);
	
	Battery::Bitmap ExportImage(bool transparent = true, float dpi = 300);

	nlohmann::json GetJson() {
		nlohmann::json j = nlohmann::json();

		nlohmann::json layers = nlohmann::json::array();
		for (Layer& layer : content.GetLayers()) {
			layers.push_back(layer.GetJson());
		}
		j["layers"] = layers;
		j["background_color"] = nlohmann::json::array({ backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a });
		j["file_type"] = JSON_FILE_TYPE;
		j["file_version"] = JSON_FILE_VERSION;

		return j;
	}
};
