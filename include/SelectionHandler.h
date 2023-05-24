#pragma once

#include "Shapes/GenericShape.h"

class SelectionHandler {

	std::vector<ShapeID> selectedShapes;
	size_t nextPossibleIndex = 0;
	ShapeID lastHoveredShape;
	ShapeID shapeBelowMouse;

public:
	SelectionHandler() {}

	const std::vector<ShapeID>& GetSelectedShapes();

	ShapeID GetHoveredShape(const ImVec2& mousePosition);
	ShapeID GetLastHoveredShape();
	void SelectNextPossibleShape();

	bool IsShapeSelected(ShapeID id);
	bool SelectShape(ShapeID id);
	bool UnselectShape(ShapeID id);
	bool ToggleSelection(ShapeID id);
	void SelectAll();
	void ClearSelection();

};
