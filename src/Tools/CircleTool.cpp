
#include "pch.h"
#include "Tools/CircleTool.h"
#include "Navigator.h"

void CircleTool::OnToolChanged() {
	Navigator::GetInstance()->previewPointShown = true;
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
}

void CircleTool::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel) {
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

void CircleTool::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {

}

void CircleTool::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {
	if (circleStarted) {
		previewCircle.SetRadius(glm::distance(snapped, previewCircle.GetCenter()));
	}

	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
}

void CircleTool::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {

}

void CircleTool::OnMouseReleased(const glm::vec2& position, bool left, bool right, bool wheel) {

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
