
#include "pch.h"
#include "Tools/LineStripTool.h"
#include "Navigator.h"

void LineStripTool::OnToolChanged() {
	Navigator::GetInstance()->previewPointShown = true;
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
}

void LineStripTool::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped) {
	if (!lineStarted) {				// Start a line
		float thickness = Navigator::GetInstance()->currentLineThickness;
		auto color = Navigator::GetInstance()->currentShapeColor;
		previewLine.SetPoint1(snapped);
		previewLine.SetPoint2(snapped);
		previewLine.SetThickness(thickness);
		previewLine.SetColor(color);
		Navigator::GetInstance()->previewPointShown = false;
		lineStarted = true;
	}
	else {						// Continue the line
		Navigator::GetInstance()->AddLine(previewLine.GetPoint1(), snapped);
		float thickness = Navigator::GetInstance()->currentLineThickness;
		auto color = Navigator::GetInstance()->currentShapeColor;
		previewLine.SetPoint1(snapped);
		previewLine.SetPoint2(snapped);
		previewLine.SetThickness(thickness);
		previewLine.SetColor(color);
		Navigator::GetInstance()->previewPointShown = false;
	}
}

void LineStripTool::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, ShapeID shape) {
}

void LineStripTool::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped) {
	if (lineStarted) {
		previewLine.SetPoint2(snapped);
	}
	else {
		Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
	}
}

void LineStripTool::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped) {
}

void LineStripTool::OnMouseReleased(const glm::vec2& position) {
}

void LineStripTool::OnLayerSelected(LayerID layer) {

}

void LineStripTool::CancelShape() {
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
	Navigator::GetInstance()->previewPointShown = true;
	lineStarted = false;
}

void LineStripTool::SelectAll() {

}

void LineStripTool::CopyClipboard() {

}

void LineStripTool::CutClipboard() {

}

void LineStripTool::PasteClipboard() {

}

void LineStripTool::RenderPreview() {
	if (lineStarted) {
		previewLine.RenderPreview();
	}
}
