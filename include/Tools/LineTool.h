#pragma once

#include "Tools/GenericTool.h"
#include "Shapes/GenericShape.h"
#include "Shapes/LineShape.h"

class LineTool : public GenericTool {

	LineShape previewLine;
	bool lineStarted = false;

public:
	LineTool() : GenericTool(ToolType::LINE), previewLine({ 0, 0 }, { 0, 0 }, 0, { 0, 0, 0, 0 }) {}

	void OnToolChanged();
	void OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, ShapeID shape);
	void OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseReleased(const glm::vec2& position);
	void OnLayerSelected(LayerID layer);
	void CancelShape();

	void SelectAll();
	void CopyClipboard();
	void CutClipboard();
	void PasteClipboard();

	void RenderPreview();

};
