#pragma once

#include "pch.h"
#include "LayerState.h"
#include "LayerHistory.h"
#include "config.h"

typedef size_t LayerID;

class Layer {

	LayerState state;
	LayerHistory<MAX_NUMBER_OF_UNDOS> history;
	LayerID id = -1;
	inline static LayerID nextID = 0;

public:
	std::string name;
	Battery::Texture2D previewImage;

	Layer(const std::string& name);

	LayerState GetState();
	void LoadState(const LayerState& state);

	LayerID GetID() const;
	void SetID(LayerID id);

	const std::vector<std::unique_ptr<GenericShape>>& GetShapes() const;

	std::unique_ptr<GenericShape> MakeShape(enum class ShapeType type, glm::vec2 p1,
		glm::vec2 p2, float thickness, const glm::vec4& color);

	void AddShape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness, const glm::vec4& color);
	bool RemoveShape(const ShapeID& id);

	bool MoveShapeLeft(const ShapeID& id, float amount);
	bool MoveShapeRight(const ShapeID& id, float amount);
	bool MoveShapeUp(const ShapeID& id, float amount);
	bool MoveShapeDown(const ShapeID& id, float amount);

	void SaveState();
	void UndoAction();

	void SetPreviewImage(const Battery::Texture2D& image);
	void GeneratePreview();

	GenericShape* FindShape(const ShapeID& shape);

	float MapFloat(float x, float in_min, float in_max, float out_min, float out_max);
	void RenderLayerToBitmap(ALLEGRO_BITMAP* bitmap);

	nlohmann::json GetJson();
};
