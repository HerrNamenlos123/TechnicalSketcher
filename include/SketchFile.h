#pragma once

#include "pch.h"
#include "config.h"
#include "FileContent.h"

/// <summary>
/// This class guarantees, that always at least 1 layer exists
/// </summary>
class SketchFile {

	FileContent content;

	bool m_fileChanged = false;
    b::fs::path m_filename = DEFAULT_FILENAME;	// Filename contains extension
	b::fs::path m_filepath = "";

public:
	ImVec4 canvasColor = DEFAULT_BACKGROUND_COLOR;

	SketchFile() {}

	void PushLayer() {
		content.PushLayer();
		m_fileChanged = true;
	}

	void PushLayer(const std::string& name) {
		content.PushLayer(name);
        m_fileChanged = true;
	}

	void PushLayer(SketchLayer&& layer) {
		content.PushLayer(std::move(layer));
        m_fileChanged = true;
	}

	void GeneratePreviews() {
		content.GeneratePreviews();
	}

	const std::vector<SketchLayer>& GetLayers() {
		return content.GetLayers();
	}

	const SketchLayer& GetActiveLayer() {
		return content.GetActiveLayer();
	}

	SketchLayer DuplicateActiveLayer() {
		return content.GetActiveLayer().Duplicate();
	}

	bool SetLayerName(LayerID id, const char* name) {
		auto layer = content.FindLayer(id);

		if (layer.has_value()) {	// Layer was found
			layer->get().name = name;
            m_fileChanged = true;
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
        m_fileChanged = true;
		return true;
	}

	bool MoveLayerBack(LayerID id) {
		if (!content.MoveLayerBack(id)) {
			return false;
		}
        m_fileChanged = true;
		return true;
	}

	bool RemoveLayer(LayerID id) {
		if (!content.RemoveLayer(id)) {
			return false;
		}
        m_fileChanged = true;
		return true;
	}

	bool ActivateLayer(LayerID id) {
		return content.ActivateLayer(id);
	}

	void AddShape(enum class ShapeType type, ImVec2 p1, ImVec2 p2, float thickness, const ImVec4& color) {
		content.GetActiveLayer().AddShape(type, p1, p2, thickness, color);
        m_fileChanged = true;
	}

	void AddShape(enum class ShapeType type, ImVec2 center, float radius, float thickness, const ImVec4& color) {
		content.GetActiveLayer().AddShape(type, center, radius, thickness, color);
        m_fileChanged = true;
	}

	void AddShape(enum class ShapeType type, ImVec2 center, float radius, float startAngle, float endAngle, float thickness, const ImVec4& color) {
		content.GetActiveLayer().AddShape(type, center, radius, startAngle, endAngle, thickness, color);
        m_fileChanged = true;
	}

	void AddShapes(std::vector<ShapePTR>&& shapes) {
		content.GetActiveLayer().AddShapes(std::move(shapes));
        m_fileChanged = true;
	}

	bool RemoveShapes(const std::vector<ShapeID>& ids) {
		if (content.GetActiveLayer().RemoveShapes(ids)) {
            m_fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesLeft(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesLeft(ids, amount)) {
            m_fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesRight(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesRight(ids, amount)) {
            m_fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesUp(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesUp(ids, amount)) {
            m_fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesDown(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesDown(ids, amount)) {
            m_fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapes(const std::vector<ShapeID>& ids, ImVec2 amount) {
		if (content.GetActiveLayer().MoveShapes(ids, amount)) {
            m_fileChanged = true;
			return true;
		}
		return false;
	}

	void ShowPropertiesWindow(ShapeID id) {
		auto opt = content.GetActiveLayer().FindShape(id);

		if (opt.has_value()) {
			if (opt.value().get().ShowPropertiesWindow()) {
                m_fileChanged = true;
			}
		}
	}

	std::optional<std::reference_wrapper<const GenericShape>> FindShape(const ShapeID& id) {
		return content.GetActiveLayer().FindShape(id);
	}

	void UpdateWindowTitle();

	void FileChanged() {
        m_fileChanged = true;
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
		return m_fileChanged;
	}

	bool SaveFile(bool saveAs = false);
	bool OpenFile();
	bool OpenEmptyFile();
	bool OpenFile(const b::fs::path& path, bool silent = false);
	
	sf::Image ExportImage(bool transparent = true, float dpi = 300);

	nlohmann::json GetJson() {
		nlohmann::json j = nlohmann::json();

		nlohmann::json layers = nlohmann::json::array();
		for (SketchLayer& layer : content.GetLayers()) {
			layers.push_back(layer.GetJson());
		}
		j["layers"] = layers;
		j["background_color"] = nlohmann::json::array({ canvasColor.x, canvasColor.y, canvasColor.z, canvasColor.w });  // TODO: Serialize ImVec4 in nlohmann::json directly
		j["file_type"] = JSON_FILE_TYPE;
		j["file_version"] = JSON_FILE_VERSION;

		return j;
	}
};
