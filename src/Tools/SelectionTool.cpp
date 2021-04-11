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

void SelectionTool::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel) {
	
	if (left) {
		// Start a selection box
		selectionBoxActive = true;
		selectionBoxPointA = position;
		selectionBoxPointB = position;

		if (!Navigator::GetInstance()->controlKeyPressed) {
			selectionHandler.ClearSelection();
		}
	}
	else if (right) {
		if (!Navigator::GetInstance()->controlKeyPressed) {
			selectionHandler.ClearSelection();
		}
	}
}

void SelectionTool::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {
	if (left) {
		if (Navigator::GetInstance()->controlKeyPressed) {
			selectionHandler.ToggleSelection(shape);
		}
		else {
			selectionHandler.ClearSelection();
			selectionHandler.SelectShape(shape);
		}
	}
}

void SelectionTool::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {
	selectionBoxActive = false;
	selectionHandler.GetHoveredShape(position);
}

void SelectionTool::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {
	// Move selection box
	if (Battery::GetApplication()->window.GetLeftMouseButton()) {
		selectionBoxPointB = position;
	} 
	else if (Battery::GetApplication()->window.GetMouseWheel()) {
		Navigator::GetInstance()->panOffset += glm::vec2(dx, dy);
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
	}
}

void SelectionTool::OnMouseReleased(const glm::vec2& position, bool left, bool right, bool wheel) {
	// Finish selection box
	if (left) {
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
		GetClientApplication()->window.SetClipboardCustomFormatString(Navigator::GetInstance()->clipboardShapeFormat, j.dump(4));
	}
	else {
		LOG_WARN("Nothing copied to clipboard: No shapes selected");
	}
}

void SelectionTool::CutClipboard() {

	if (selectionHandler.GetSelectedShapes().size() != 0) {
		LOG_INFO("Cutting selected shapes to clipboard");
		nlohmann::json j = Navigator::GetInstance()->file.GetJsonFromShapes(selectionHandler.GetSelectedShapes());
		GetClientApplication()->window.SetClipboardCustomFormatString(Navigator::GetInstance()->clipboardShapeFormat, j.dump(4));
		Navigator::GetInstance()->file.RemoveShapes(selectionHandler.GetSelectedShapes());
		selectionHandler.ClearSelection();
	}
	else {
		LOG_WARN("Nothing cut to clipboard: No shapes selected");
	}
}

void SelectionTool::PasteClipboard() {

	auto opt = GetClientApplication()->window.GetClipboardCustomFormatString(Navigator::GetInstance()->clipboardShapeFormat);

	if (!opt.has_value()) {
		LOG_WARN("Nothing usable on the clipboard");
		return;
	}

	LOG_INFO("Pasting clipboard to active Layer");
	
	// First create all shapes
	std::vector<ShapePTR> shapes;
	try {
		nlohmann::json j = nlohmann::json::parse(opt.value());
	
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
