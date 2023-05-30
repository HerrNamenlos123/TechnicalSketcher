
#include "pch.h"
#include "Tools/CircleTool.h"
#include "Navigator.h"

void CircleTool::OnToolChanged() {
	Navigator::GetInstance()->previewPointShown = true;
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
}

void CircleTool::OnSpaceClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel) {
	if (left) {
		if (!circleStarted) {				// Start a line
			previewCircle.SetCenter(snapped);
			previewCircle.SetRadius(0);
			Navigator::GetInstance()->previewPointShown = true;
			circleStarted = true;
		}
		else {						// Finish a line
			Navigator::GetInstance()->AddCircle(previewCircle);
			CancelShape();
		}
	}
	else {
		CancelShape();
	}
}

void CircleTool::OnShapeClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {

}

void CircleTool::OnMouseHovered(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
	if (circleStarted) {
		previewCircle.SetRadius(b::distance(snapped, previewCircle.GetCenter()));
	}

	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
}

void CircleTool::OnMouseDragged(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {

}

void CircleTool::OnMouseReleased(const ImVec2& position, bool left, bool right, bool wheel) {

}

void CircleTool::OnLayerSelected(LayerID layer) {

}

void CircleTool::CancelShape() {
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
	Navigator::GetInstance()->previewPointShown = true;
	circleStarted = false;
}

void CircleTool::SelectAll() {

}

void CircleTool::CopyClipboard() {

}

void CircleTool::CutClipboard() {

}

void CircleTool::PasteClipboard() {

}

bool CircleTool::StepToolBack() {
	if (circleStarted) {
		CancelShape();
		return true;
	}

	return false;
}

bool CircleTool::IsPropertiesWindowShown() {
	if (!circleStarted) {
		return true;
	}

	return false;
}

void CircleTool::ShowPropertiesWindow() {
	if (!circleStarted) {
		previewCircle.ShowPropertiesWindow();
	}
}

void CircleTool::RenderPreview() {
	if (circleStarted) {
		previewCircle.RenderPreview();
	}
}
