
#include "pch.h"
#include "Tools/LineTool.h"
#include "Shapes/LineShape.h"
#include "Navigator.h"

void LineTool::OnToolChanged() {
	Navigator::GetInstance()->previewPointShown = true;
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
}

void LineTool::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel) {
	if (left) {
		if (!lineStarted) {				// Start a line
			previewLine.SetPoint1(snapped);
			previewLine.SetPoint2(snapped);
			Navigator::GetInstance()->previewPointShown = false;
			lineStarted = true;
		}
		else {						// Finish a line
			Navigator::GetInstance()->AddLine(previewLine);
			CancelShape();
		}
	}
	else {
		CancelShape();
	}
}

void LineTool::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {

}

void LineTool::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {
	if (lineStarted) {
		previewLine.SetPoint2(snapped);
	}
	else {
		Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
	}
}

void LineTool::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {

}

void LineTool::OnMouseReleased(const glm::vec2& position, bool left, bool right, bool wheel) {

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

bool LineTool::IsPropertiesWindowShown() {
	if (!lineStarted) {
		return true;
	}

	return false;
}

void LineTool::ShowPropertiesWindow() {
	if (!lineStarted) {
		previewLine.ShowPropertiesWindow();
	}
}

void LineTool::RenderPreview() {
	if (lineStarted) {
		previewLine.RenderPreview();
	}
}
