
#include "pch.h"
#include "SelectionHandler.h"
#include "Navigator.h"

const std::vector<ShapeID>& SelectionHandler::GetSelectedShapes() {

	std::vector<ShapeID> removeSelection;
	for (ShapeID id : selectedShapes) {
		if (!Navigator::GetInstance()->m_file.GetActiveLayer().ShapeExists(id)) {
			removeSelection.push_back(id);
		}
	}

	// Now remove all non-existing shapes
	for (ShapeID id : removeSelection) {
		UnselectShape(id);
	}

	return selectedShapes;
}



ShapeID SelectionHandler::GetHoveredShape(const ImVec2& mousePosition) {
	std::vector<ShapeID> possible;

	for (const auto& shape : Navigator::GetInstance()->m_file.GetActiveLayer().GetShapes()) {
		float dist = Navigator::GetInstance()->ConvertScreenToWorkspaceDistance(
			Navigator::GetInstance()->mouseHighlightThresholdDistance);
		if (shape->IsShapeHovered(mousePosition, dist)) {
			possible.push_back(shape->GetID());
		}
	}

	// Reset index if out of bounds
	if (nextPossibleIndex >= possible.size()) {
		nextPossibleIndex = 0;
	}

	if (possible.size() > 0) {
		lastHoveredShape = possible[nextPossibleIndex];
	}
	else {
		lastHoveredShape = -1;
	}

	return lastHoveredShape;
}

ShapeID SelectionHandler::GetLastHoveredShape() {
	return lastHoveredShape;
}

void SelectionHandler::SelectNextPossibleShape() {
	nextPossibleIndex++;
	GetHoveredShape(Navigator::GetInstance()->mousePosition);
}




bool SelectionHandler::IsShapeSelected(ShapeID id) {

	for (ShapeID shapeID : selectedShapes) {
		if (shapeID == id) {
			return true;
		}
	}

	return false;
}

bool SelectionHandler::SelectShape(ShapeID id) {

	if (Navigator::GetInstance()->m_file.GetActiveLayer().ShapeExists(id)) {

		// Shape exists, select it now if it's not already
		if (!IsShapeSelected(id)) {
			selectedShapes.push_back(id);
		}
		return true;
	}

	return false;
}

bool SelectionHandler::UnselectShape(ShapeID id) {

	for (size_t i = 0; i < selectedShapes.size(); i++) {
		if (selectedShapes[i] == id) {
			selectedShapes.erase(selectedShapes.begin() + i);
			return true;
		}
	}

	return false;
}

bool SelectionHandler::ToggleSelection(ShapeID id) {

	if (IsShapeSelected(id)) {
		return UnselectShape(id);
	}
	else {
		return SelectShape(id);
	}
}

void SelectionHandler::SelectAll() {
	selectedShapes.clear();

	for (const auto& shape : Navigator::GetInstance()->m_file.GetActiveLayer().GetShapes()) {
		selectedShapes.push_back(shape->GetID());
	}
}

void SelectionHandler::ClearSelection() {
	selectedShapes.clear();
}
