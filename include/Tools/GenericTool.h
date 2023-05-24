#pragma once

#include "SketchLayer.h"
#include "Shapes/GenericShape.h"

enum class ToolType {
	NONE,
	SELECT,
	LINE,
	LINE_STRIP,
	CIRCLE,
	ARC
};

class GenericTool {

	enum class ToolType type;

public:
	GenericTool(enum class ToolType type) : type(type) {}

	enum class ToolType GetType() {
		return type;
	}

	virtual void OnToolChanged() = 0;
	virtual void OnSpaceClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel) = 0;
	virtual void OnShapeClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel, ShapeID shape) = 0;
	virtual void OnMouseHovered(const ImVec2& position, const ImVec2& snapped, float dx, float dy) = 0;
	virtual void OnMouseDragged(const ImVec2& position, const ImVec2& snapped, float dx, float dy) = 0;
	virtual void OnMouseReleased(const ImVec2& position, bool left, bool right, bool wheel) = 0;
	virtual void OnLayerSelected(LayerID layer) = 0;
	virtual void CancelShape() = 0;

	virtual void SelectAll() = 0;
	virtual void CopyClipboard() = 0;
	virtual void CutClipboard() = 0;
	virtual void PasteClipboard() = 0;
	virtual bool StepToolBack() = 0;

	virtual bool IsPropertiesWindowShown() = 0;
	virtual void ShowPropertiesWindow() = 0;

	virtual void RenderPreview() = 0;

};
