#pragma once

#include "config.h"
#include "Tools/GenericTool.h"
#include "Shapes/LineShape.h"

class LineTool : public GenericTool {

	LineShape previewLine;
	bool lineStarted = false;
	
public:
	LineTool() : GenericTool(ToolType::LINE), previewLine({ 0, 0 }, { 0, 0 }, DEFAULT_LINE_THICKNESS, DEFAULT_LINE_COLOR) {}

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
