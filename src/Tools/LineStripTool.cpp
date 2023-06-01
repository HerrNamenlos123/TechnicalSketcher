//
//#include "pch.h"
//#include "Tools/LineStripTool.h"
//#include "Navigator.h"
//
//void LineStripTool::OnToolChanged() {
//	Navigator::GetInstance()->previewPointShown = true;
//	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
//}
//
//void LineStripTool::OnSpaceClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel) {
//	if (!lineStarted && left) {				// Start a line
//		previewLine.SetPoint1(snapped);
//		previewLine.SetPoint2(snapped);
//		Navigator::GetInstance()->previewPointShown = false;
//		lineStarted = true;
//	}
//	else if (left) {						// Continue the line
//		previewLine.SetPoint2(snapped);
//		Navigator::GetInstance()->AddLine(previewLine);
//		previewLine.SetPoint1(snapped);
//		Navigator::GetInstance()->previewPointShown = false;
//	}
//	else {
//		CancelShape();
//	}
//}
//
//void LineStripTool::OnShapeClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {
//}
//
//void LineStripTool::OnMouseHovered(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
//	if (lineStarted) {
//		previewLine.SetPoint2(snapped);
//	}
//	else {
//		Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
//	}
//}
//
//void LineStripTool::OnMouseDragged(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
//}
//
//void LineStripTool::OnMouseReleased(const ImVec2& position, bool left, bool right, bool wheel) {
//}
//
//void LineStripTool::OnLayerSelected(LayerID layer) {
//
//}
//
//void LineStripTool::CancelShape() {
//	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
//	Navigator::GetInstance()->previewPointShown = true;
//	lineStarted = false;
//}
//
//void LineStripTool::SelectAll() {
//
//}
//
//void LineStripTool::CopyClipboard() {
//
//}
//
//void LineStripTool::CutClipboard() {
//
//}
//
//void LineStripTool::PasteClipboard() {
//
//}
//
//bool LineStripTool::StepToolBack() {
//	if (lineStarted) {
//		CancelShape();
//		return true;
//	}
//
//	return false;
//}
//
//bool LineStripTool::IsPropertiesWindowShown() {
//	if (!lineStarted) {
//		return true;
//	}
//
//	return false;
//}
//
//void LineStripTool::ShowPropertiesWindow() {
//	if (!lineStarted) {
//		previewLine.ShowPropertiesWindow();
//	}
//}
//
//void LineStripTool::RenderPreview() {
//	if (lineStarted) {
//		previewLine.RenderPreview();
//	}
//}
