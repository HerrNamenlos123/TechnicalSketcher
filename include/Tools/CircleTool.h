//#pragma once
//
//#include "config.h"
//#include "Tools/GenericTool.h"
//#include "Shapes/GenericShape.h"
//#include "Shapes/CircleShape.h"
//
//class CircleTool : public GenericTool {
//
//	CircleShape previewCircle;
//	bool circleStarted = false;
//
//public:
//	CircleTool() : GenericTool(ToolType::CIRCLE), previewCircle({ 0, 0 }, 0, DEFAULT_LINE_THICKNESS, DEFAULT_LINE_COLOR) {}
//
//	void OnToolChanged();
//	void OnSpaceClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel);
//	void OnShapeClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel, ShapeID shape);
//	void OnMouseHovered(const ImVec2& position, const ImVec2& snapped, int dx, int dy);
//	void OnMouseDragged(const ImVec2& position, const ImVec2& snapped, int dx, int dy);
//	void OnMouseReleased(const ImVec2& position, bool left, bool right, bool wheel);
//	void OnLayerSelected(LayerID layer);
//	void CancelShape();
//
//	void SelectAll();
//	void CopyClipboard();
//	void CutClipboard();
//	void PasteClipboard();
//	bool StepToolBack();
//
//	bool IsPropertiesWindowShown();
//	void ShowPropertiesWindow();
//
//	void RenderPreview();
//
//};