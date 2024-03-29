//#pragma once
//
//#include "pch.h"
//#include "LayerState.h"
//#include "LayerHistory.h"
//#include "config.h"
//
//typedef size_t LayerID;
//
//class SketchLayer {
//
//	LayerState state;
//	LayerHistory<MAX_NUMBER_OF_UNDOS> history;
//	LayerID id = -1;
//	inline static LayerID nextID = 0;
//
//public:
//	std::string name;
//	//sf::RenderTexture previewImage;
//	bool layerChanged = false;
//
//	SketchLayer(const std::string& name);
//
//	LayerState GetState();
//	void LoadState(const LayerState& state);
//	SketchLayer Duplicate();
//
//	LayerID GetID() const;
//	void SetID(LayerID id);
//
//	const std::vector<ShapePTR>& GetShapes() const;
//
//	bool AddShape(const nlohmann::json& json);
//	void AddShape(enum class ShapeType type, ImVec2 p1, ImVec2 p2, float thickness, const ImVec4& color);
//	void AddShape(enum class ShapeType type, ImVec2 center, float radius, float thickness, const ImVec4& color);
//	void AddShape(enum class ShapeType type, ImVec2 center, float radius, float startAngle, float endAngle, float thickness, const ImVec4& color);
//	bool AddShapes(const std::vector<nlohmann::json>& jsonArray);
//	void AddShapes(std::vector<ShapePTR>&& shapes);
//	bool RemoveShape(const ShapeID& id);
//	bool RemoveShapes(const std::vector<ShapeID>& ids);
//
//	bool MoveShapeLeft(const ShapeID& id, float amount);
//	bool MoveShapeRight(const ShapeID& id, float amount);
//	bool MoveShapeUp(const ShapeID& id, float amount);
//	bool MoveShapeDown(const ShapeID& id, float amount);
//	bool MoveShapesLeft(const std::vector<ShapeID>& ids, float amount);
//	bool MoveShapesRight(const std::vector<ShapeID>& ids, float amount);
//	bool MoveShapesUp(const std::vector<ShapeID>& ids, float amount);
//	bool MoveShapesDown(const std::vector<ShapeID>& ids, float amount);
//	bool MoveShapes(const std::vector<ShapeID>& ids, ImVec2 amount);
//
//	void SaveState();
//	void UndoAction();
//
//	//void SetPreviewImage(const sf::Image& image);
//	//void GeneratePreview();
//
//	std::optional<std::reference_wrapper<GenericShape>> FindShape(const ShapeID& shape);
//	bool ShapeExists(const ShapeID& id) const;
//	std::pair<ImVec2, ImVec2> GetBoundingBox() const;
//
//	float MapFloat(float x, float in_min, float in_max, float out_min, float out_max);
//	void RenderLayerToBitmap(sf::RenderTexture texture);
//
//	bool LoadJson(nlohmann::json json);
//	nlohmann::json GetJson();
//};
