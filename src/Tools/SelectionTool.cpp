#pragma once

#include "pch.h"
#include "Tools/SelectionTool.h"
#include "Navigator.h"
#include "Application.h"
#include "Shapes/GenericShape.h"

void SelectionTool::OnToolChanged() {
	Navigator::GetInstance()->previewPointShown = false;
	selectionBoxActive = false;
}

void SelectionTool::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped) {
	// Start a selection box
	selectionBoxActive = true;
	selectionBoxPointA = position;
	selectionBoxPointB = position;

	if (!Navigator::GetInstance()->controlKeyPressed) {
		selectionHandler.ClearSelection();
	}
}

void SelectionTool::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, ShapeID shape) {
	if (Navigator::GetInstance()->controlKeyPressed) {
		selectionHandler.ToggleSelection(shape);
	}
	else {
		selectionHandler.ClearSelection();
		selectionHandler.SelectShape(shape);
	}
}

void SelectionTool::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped) {
	selectionBoxActive = false;
	selectionHandler.GetHoveredShape(position);
}

void SelectionTool::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped) {
	// Move selection box
	selectionBoxPointB = position;
}

void SelectionTool::OnMouseReleased(const glm::vec2& position) {
	// Finish selection box
	if (selectionBoxActive) {
		selectionBoxActive = false;

		// If CTRL key is pressed, don't delete previously selected shapes
		if (!Navigator::GetInstance()->controlKeyPressed) {
			selectionHandler.ClearSelection();
		}

		// Select all shapes inside the selection box
		for (const auto& shape : Navigator::GetInstance()->file.GetActiveLayer().GetShapes()) {
			if (shape->IsInSelectionBox(selectionBoxPointA, selectionBoxPointB)) {
				selectionHandler.SelectShape(shape->GetID());
			}
		}
	}
}

void SelectionTool::OnLayerSelected(LayerID layer) {

}

void SelectionTool::CancelShape() {

}

void SelectionTool::SelectAll() {
	selectionHandler.SelectAll();
}

void SelectionTool::CopyClipboard() {

	if (selectionHandler.GetSelectedShapes().size() != 0) {
		LOG_INFO("Copying selected shapes to clipboard");
		nlohmann::json j = Navigator::GetInstance()->file.GetJsonFromShapes(selectionHandler.GetSelectedShapes());
		GetClientApplication()->window.SetClipboardContent(j.dump(4));
	}
	else {
		LOG_WARN("Nothing copied to clipboard: No shapes selected");
	}
}

void SelectionTool::CutClipboard() {

	if (selectionHandler.GetSelectedShapes().size() != 0) {
		LOG_INFO("Cutting selected shapes to clipboard");
		nlohmann::json j = Navigator::GetInstance()->file.GetJsonFromShapes(selectionHandler.GetSelectedShapes());
		GetClientApplication()->window.SetClipboardContent(j.dump(4));
		Navigator::GetInstance()->file.RemoveShapes(selectionHandler.GetSelectedShapes());
		selectionHandler.ClearSelection();
	}
	else {
		LOG_WARN("Nothing cut to clipboard: No shapes selected");
	}
}

void SelectionTool::PasteClipboard() {

	LOG_INFO("Pasting clipboard to active Layer");
	
	// First create all shapes
	std::vector<ShapePTR> shapes;
	try {
		nlohmann::json j = nlohmann::json::parse(GetClientApplication()->window.GetClipboardContent());
	
		for (nlohmann::json shapeData : j) {
			shapes.push_back(GenericShape::MakeShape(shapeData));
		}
	}
	catch (...) {
		LOG_ERROR("Can't paste clipboard shapes: JSON format is invalid!");
		return;
	}

	// Calculate the average center position as long as they're here
	glm::vec2 averagePos = { 0, 0 };
	for (ShapePTR& shape : shapes) {
		averagePos += shape->GetCenterPosition();
	}
	averagePos /= shapes.size();
	
	// Save the ids of the shapes
	std::vector<ShapeID> ids;
	for (ShapePTR& shape : shapes) {
		ids.push_back(shape->GetID());
	}

	// Move the shapes to the mouse position
	glm::vec2 moveAmount = Navigator::GetInstance()->mousePosition - averagePos;
	float snap = Navigator::GetInstance()->snapSize;
	moveAmount = round(moveAmount / snap) * snap;
	for (ShapePTR& shape : shapes) {
		shape->Move(moveAmount);
	}
	
	// Now put shapes into file
	Navigator::GetInstance()->file.AddShapes(std::move(shapes));
	
	// Finally, select all just added shapes
	selectionHandler.ClearSelection();
	for (ShapeID& id : ids) {
		selectionHandler.SelectShape(id);
	}
}

bool SelectionTool::StepToolBack() {
	return false;	// false: can't step back further
}

void SelectionTool::RenderFirstPart() {
	if (selectionBoxActive) {			// Fill should be in the back
		ApplicationRenderer::DrawSelectionBoxInfillRectangle(selectionBoxPointA, selectionBoxPointB);
	}
}

void SelectionTool::RenderSecondPart() {
	if (selectionBoxActive) {			// Fill should be in the back
		ApplicationRenderer::DrawSelectionBoxOutlineRectangle(selectionBoxPointA, selectionBoxPointB);
	}
}

void SelectionTool::RemoveSelectedShapes() {
	Navigator::GetInstance()->file.RemoveShapes(selectionHandler.GetSelectedShapes());
	selectionHandler.ClearSelection();
}

void SelectionTool::MoveSelectedShapesLeft(float amount) {
	Navigator::GetInstance()->file.MoveShapesLeft(selectionHandler.GetSelectedShapes(), amount);
}

void SelectionTool::MoveSelectedShapesRight(float amount) {
	Navigator::GetInstance()->file.MoveShapesRight(selectionHandler.GetSelectedShapes(), amount);
}

void SelectionTool::MoveSelectedShapesUp(float amount) {
	Navigator::GetInstance()->file.MoveShapesUp(selectionHandler.GetSelectedShapes(), amount);
}

void SelectionTool::MoveSelectedShapesDown(float amount) {
	Navigator::GetInstance()->file.MoveShapesDown(selectionHandler.GetSelectedShapes(), amount);
}

void SelectionTool::SelectNextPossibleShape() {
	selectionHandler.SelectNextPossibleShape();
}

void SelectionTool::RenderPreview() {

}
