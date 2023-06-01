//#pragma once
//
//#include "Tools/GenericTool.h"
//#include "SelectionHandler.h"
//
//class SelectionTool : public GenericTool {
//public:
//
//	ImVec2 selectionBoxPointA = { 0, 0 };
//	ImVec2 selectionBoxPointB = { 0, 0 };
//	bool selectionBoxActive = false;
//	SelectionHandler selectionHandler;
//
//	SelectionTool() : GenericTool(ToolType::SELECT) {}
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
//	void RenderFirstPart();
//	void RenderSecondPart();
//	void RemoveSelectedShapes();
//	void MoveSelectedShapesLeft(float amount);
//	void MoveSelectedShapesRight(float amount);
//	void MoveSelectedShapesUp(float amount);
//	void MoveSelectedShapesDown(float amount);
//	void SelectNextPossibleShape();
//
//	void RenderPreview();
//
//};
