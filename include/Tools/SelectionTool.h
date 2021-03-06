#pragma once

#include "Tools/GenericTool.h"
#include "SelectionHandler.h"

class SelectionTool : public GenericTool {
public:

	glm::vec2 selectionBoxPointA = { 0, 0 };
	glm::vec2 selectionBoxPointB = { 0, 0 };
	bool selectionBoxActive = false;
	SelectionHandler selectionHandler;

	SelectionTool() : GenericTool(ToolType::SELECT) {}

	void OnToolChanged();
	void OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel);
	void OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel, ShapeID shape);
	void OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy);
	void OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy);
	void OnMouseReleased(const glm::vec2& position, bool left, bool right, bool wheel);
	void OnLayerSelected(LayerID layer);
	void CancelShape();

	void SelectAll();
	void CopyClipboard();
	void CutClipboard();
	void PasteClipboard();
	bool StepToolBack();

	bool IsPropertiesWindowShown();
	void ShowPropertiesWindow();

	void RenderFirstPart();
	void RenderSecondPart();
	void RemoveSelectedShapes();
	void MoveSelectedShapesLeft(float amount);
	void MoveSelectedShapesRight(float amount);
	void MoveSelectedShapesUp(float amount);
	void MoveSelectedShapesDown(float amount);
	void SelectNextPossibleShape();

	void RenderPreview();

};
