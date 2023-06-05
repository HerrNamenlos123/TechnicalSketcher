#pragma once

#include <vector>
#include "Shapes/GenericShape.h"

typedef size_t LayerID;

class LayerState {

	std::vector<ShapePTR> shapes;

public:
	LayerState();
	LayerState(const LayerState& state);
	void operator=(const LayerState& state);

	void PushShape(ShapePTR&& shape);
	bool RemoveShape(ShapeID id);
	std::optional<std::reference_wrapper<GenericShape>> FindShape(ShapeID id);
	bool ShapeExists(const ShapeID& id) const;
	std::pair<ImVec2, ImVec2> GetBoundingBox() const;

	const std::vector<ShapePTR>& GetShapes() const;

	bool LoadJson(nlohmann::json json);
	nlohmann::json GetJson();
};
