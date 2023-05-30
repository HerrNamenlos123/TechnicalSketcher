#pragma once

#include "pch.h"
#include "Tools/SelectionTool.h"
#include "Navigator.h"
#include "Shapes/GenericShape.h"

void SelectionTool::OnToolChanged() {
	Navigator::GetInstance()->previewPointShown = false;
	selectionBoxActive = false;
	selectionHandler.ClearSelection();
}

void SelectionTool::OnSpaceClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel) {
	
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

void SelectionTool::OnShapeClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {
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

void SelectionTool::OnMouseHovered(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
	selectionBoxActive = false;
	selectionHandler.GetHoveredShape(position);
}

void SelectionTool::OnMouseDragged(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
	// Move selection box
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		selectionBoxPointB = position;
	} 
	else if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
		Navigator::GetInstance()->m_panOffset += ImVec2(static_cast<float>(dx), static_cast<float>(dy));
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
	}
}

void SelectionTool::OnMouseReleased(const ImVec2& position, bool left, bool right, bool wheel) {
	// Finish selection box
	if (left) {
		if (selectionBoxActive) {
			selectionBoxActive = false;

			// If CTRL key is pressed, don't delete previously selected shapes
			if (!Navigator::GetInstance()->controlKeyPressed) {
				selectionHandler.ClearSelection();
			}

			// Select all shapes inside the selection box
			for (const auto& shape : Navigator::GetInstance()->m_file.GetActiveLayer().GetShapes()) {
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
		b::log::info("Copying selected shapes to clipboard");
		nlohmann::json j = Navigator::GetInstance()->m_file.GetJsonFromShapes(selectionHandler.GetSelectedShapes());
		//Battery::GetMainWindow().SetClipboardCustomFormatString(Navigator::GetInstance()->clipboardShapeFormat, j.dump(4));
		b::log::error("CLIPBOARD MISSING");
	}
	else {
		b::log::warn("Nothing copied to clipboard: No shapes selected");
	}
}

void SelectionTool::CutClipboard() {

	if (selectionHandler.GetSelectedShapes().size() != 0) {
		b::log::info("Cutting selected shapes to clipboard");
		nlohmann::json j = Navigator::GetInstance()->m_file.GetJsonFromShapes(selectionHandler.GetSelectedShapes());
		//Battery::GetMainWindow().SetClipboardCustomFormatString(Navigator::GetInstance()->clipboardShapeFormat, j.dump(4));
		Navigator::GetInstance()->m_file.RemoveShapes(selectionHandler.GetSelectedShapes());
		selectionHandler.ClearSelection();
	}
	else {
		b::log::warn("Nothing cut to clipboard: No shapes selected");
	}
}

void SelectionTool::PasteClipboard() {

	/*auto opt = Battery::GetMainWindow().GetClipboardCustomFormatString(Navigator::GetInstance()->clipboardShapeFormat);

	if (!opt.has_value()) {
		b::log::warn("Nothing usable on the clipboard");
		return;
	}

	b::log::info("Pasting clipboard to active Layer");
	
	// First create all shapes
	std::vector<ShapePTR> shapes;
	try {
		nlohmann::json j = nlohmann::json::parse(opt.value());
	
		for (nlohmann::json shapeData : j) {
			shapes.push_back(GenericShape::MakeShape(shapeData));
		}
	}
	catch (...) {
		b::log::error("Can't paste clipboard shapes: JSON format is invalid!");
		return;
	}
	
	// Calculate the average center position as long as they're here
	ImVec2 averagePos = { 0, 0 };
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
	ImVec2 moveAmount = Navigator::GetInstance()->mousePosition - averagePos;
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
	}*/
}

bool SelectionTool::StepToolBack() {
	return false;	// false: can't step back further
}

bool SelectionTool::IsPropertiesWindowShown() {
	// Show properties window if exactly 1 shape is selected
	if (selectionHandler.GetSelectedShapes().size() == 1) {
		return true;
	}

	return false;
}

void SelectionTool::ShowPropertiesWindow() {
	if (selectionHandler.GetSelectedShapes().size() == 1) {
		ShapeID id = selectionHandler.GetSelectedShapes()[0];
		Navigator::GetInstance()->m_file.ShowPropertiesWindow(id);
	}
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
	Navigator::GetInstance()->m_file.RemoveShapes(selectionHandler.GetSelectedShapes());
	selectionHandler.ClearSelection();
}

void SelectionTool::MoveSelectedShapesLeft(float amount) {
	Navigator::GetInstance()->m_file.MoveShapesLeft(selectionHandler.GetSelectedShapes(), amount);
}

void SelectionTool::MoveSelectedShapesRight(float amount) {
	Navigator::GetInstance()->m_file.MoveShapesRight(selectionHandler.GetSelectedShapes(), amount);
}

void SelectionTool::MoveSelectedShapesUp(float amount) {
	Navigator::GetInstance()->m_file.MoveShapesUp(selectionHandler.GetSelectedShapes(), amount);
}

void SelectionTool::MoveSelectedShapesDown(float amount) {
	Navigator::GetInstance()->m_file.MoveShapesDown(selectionHandler.GetSelectedShapes(), amount);
}

void SelectionTool::SelectNextPossibleShape() {
	selectionHandler.SelectNextPossibleShape();
}

void SelectionTool::RenderPreview() {

}
