
#include "pch.h"
#include "Tools/LineTool.h"
#include "Shapes/LineShape.h"
#include "Navigator.h"

void LineTool::OnToolChanged() {
	Navigator::GetInstance()->previewPointShown = true;
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
}

void LineTool::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped) {
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
	else {						// Finish a line
		Navigator::GetInstance()->AddLine(previewLine.GetPoint1(), snapped);
		CancelShape();
	}
}

void LineTool::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, ShapeID shape) {

}

void LineTool::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped) {
	if (lineStarted) {
		previewLine.SetPoint2(snapped);
	}
	else {
		Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
	}
}

void LineTool::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped) {

}

void LineTool::OnMouseReleased(const glm::vec2& position) {

}

void LineTool::OnLayerSelected(LayerID layer) {

}

void LineTool::CancelShape() {
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
	Navigator::GetInstance()->previewPointShown = true;
	lineStarted = false;
}

void LineTool::SelectAll() {

}

void LineTool::CopyClipboard() {

}

void LineTool::CutClipboard() {

}

void LineTool::PasteClipboard() {

}

bool LineTool::StepToolBack() {
	if (lineStarted) {
		CancelShape();
		return true;
	}

	return false;
}

void LineTool::RenderPreview() {
	if (lineStarted) {
		previewLine.RenderPreview();
	}
}
