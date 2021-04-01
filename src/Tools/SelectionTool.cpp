#pragma once

#include "pch.h"
#include "Tools/SelectionTool.h"
#include "Navigator.h"
#include "Application.h"

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

	LOG_INFO("Copying selected shapes to clipboard");
	nlohmann::json j = nlohmann::json();
	
	// Copy all selected shapes to the clipboard
	for (auto id : selectionHandler.GetSelectedShapes()) {
		auto shape = Navigator::GetInstance()->file.GetActiveLayer().FindShape(id);

		if (shape) {
			j.push_back(shape->GetJson());
		}
	}

	GetClientApplication()->window.SetClipboardContent(j.dump(4));
}

void SelectionTool::CutClipboard() {
	
}

void SelectionTool::PasteClipboard() {

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
	Navigator::GetInstance()->file.GetActiveLayer().SaveState();
	for (auto id : selectionHandler.GetSelectedShapes()) {
		Navigator::GetInstance()->file.GetActiveLayer().RemoveShape(id);
	}
	selectionHandler.ClearSelection();
}

void SelectionTool::MoveSelectedShapesLeft(float amount) {
	Navigator::GetInstance()->file.GetActiveLayer().SaveState();
	for (auto id : selectionHandler.GetSelectedShapes()) {
		Navigator::GetInstance()->file.GetActiveLayer().MoveShapeLeft(id, amount);
	}
}

void SelectionTool::MoveSelectedShapesRight(float amount) {
	Navigator::GetInstance()->file.GetActiveLayer().SaveState();
	for (auto id : selectionHandler.GetSelectedShapes()) {
		Navigator::GetInstance()->file.GetActiveLayer().MoveShapeRight(id, amount);
	}
}

void SelectionTool::MoveSelectedShapesUp(float amount) {
	Navigator::GetInstance()->file.GetActiveLayer().SaveState();
	for (auto id : selectionHandler.GetSelectedShapes()) {
		Navigator::GetInstance()->file.GetActiveLayer().MoveShapeUp(id, amount);
	}
}

void SelectionTool::MoveSelectedShapesDown(float amount) {
	Navigator::GetInstance()->file.GetActiveLayer().SaveState();
	for (auto id : selectionHandler.GetSelectedShapes()) {
		Navigator::GetInstance()->file.GetActiveLayer().MoveShapeDown(id, amount);
	}
}

void SelectionTool::SelectNextPossibleShape() {
	selectionHandler.SelectNextPossibleShape();
}

void SelectionTool::RenderPreview() {

}
