#pragma once

#include <vector>
#include "Shapes/GenericShape.h"

typedef size_t LayerID;

class LayerState {

	std::vector<std::unique_ptr<GenericShape>> shapes;

public:
	LayerState();
	LayerState(const LayerState& state);
	void operator=(const LayerState& state);
	
	void PushShape(std::unique_ptr<GenericShape>&& shape);
	bool RemoveShape(ShapeID id);
	GenericShape* FindShape(ShapeID id);
	
	const std::vector<std::unique_ptr<GenericShape>>& GetShapes() const;
};
