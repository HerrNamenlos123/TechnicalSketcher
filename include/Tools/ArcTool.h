#pragma once

#include "config.h"
#include "Tools/GenericTool.h"
#include "Shapes/GenericShape.h"
#include "Shapes/ArcShape.h"
#include "Shapes/CircleShape.h"

class ArcTool : public GenericTool {

	ArcShape previewArc;
	CircleShape previewCircle;
	bool arcStarted = false;
	bool arcSecondStage = false;

public:
	ArcTool() : GenericTool(ToolType::ARC), previewArc({ 0, 0 }, 0, 0, 0, DEFAULT_LINE_THICKNESS, DEFAULT_LINE_COLOR) {}

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

	void RenderPreview();
};