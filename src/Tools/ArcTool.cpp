
#include "pch.h"
#include "Tools/ArcTool.h"
#include "Navigator.h"

static float mag(const glm::vec2& v) {
	return sqrt(pow(v.x, 2) + pow(v.y, 2));
}

//
// double angle(const glm::vec2& v);
//
// Angle from X-Axis to vector, range 0 to 360 counterclockwise
//
//       Y
//       |     P
//       |    /
//       |   /
//       |  / \.
//       | / a |
//       |------------------X
//
static float angle(const glm::vec2& v) {
	if (mag(v) > 0) {
		if (v.y >= 0) {
			return glm::degrees(acos(v.x / mag(v)));
		}
		else if (v.y <= 0) {
			return 360 - glm::degrees(acos(v.x / mag(v)));
		}
	}

	return 0;
}




void ArcTool::OnToolChanged() {
	Navigator::GetInstance()->previewPointShown = true;
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
}

void ArcTool::OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel) {
	if (left) {
		if (!arcStarted) {			// Start the arc
			previewArc.SetCenter(snapped);
			previewArc.SetRadius(0);
			previewCircle.SetCenter(snapped);
			previewArc.SetRadius(0);
			previewCircle.SetThickness(1);
			previewCircle.SetColor({ 0, 0, 0, 255 });
			Navigator::GetInstance()->previewPointShown = true;
			arcStarted = true;
			arcSecondStage = false;
		}
		else {						// Continue the arc
			if (!arcSecondStage) {
				arcSecondStage = true;
				previewArc.SetRadius(glm::distance(snapped, previewArc.GetCenter()));

				glm::vec2 toMouse = Navigator::GetInstance()->mouseSnapped - previewArc.GetCenter();
				toMouse.y *= -1;
				previewArc.SetStartAngle(angle(toMouse));
			}
			else {				// Finish the arc
				Navigator::GetInstance()->AddArc(previewArc);
				CancelShape();
			}
		}
	}
	else {
		CancelShape();
	}
}

void ArcTool::OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {

}

void ArcTool::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {
	if (arcStarted) {
		if (!arcSecondStage) {	// First stage
			previewCircle.SetRadius(glm::distance(snapped, previewArc.GetCenter()));
		}
		else {					// Second stage
			glm::vec2 toMouse = Navigator::GetInstance()->mouseSnapped - previewArc.GetCenter();
			toMouse.y *= -1;
			previewArc.SetEndAngle(angle(toMouse));
		}
	}

	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
}

void ArcTool::OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy) {

}

void ArcTool::OnMouseReleased(const glm::vec2& position, bool left, bool right, bool wheel) {

}

void ArcTool::OnLayerSelected(LayerID layer) {

}

void ArcTool::CancelShape() {
	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
	Navigator::GetInstance()->previewPointShown = true;
	arcStarted = false;
	arcSecondStage = false;
}

void ArcTool::SelectAll() {

}

void ArcTool::CopyClipboard() {

}

void ArcTool::CutClipboard() {

}

void ArcTool::PasteClipboard() {

}

bool ArcTool::StepToolBack() {
	if (arcStarted) {
		CancelShape();
		return true;
	}

	return false;
}

bool ArcTool::IsPropertiesWindowShown() {
	if (!arcStarted) {
		return true;
	}

	return false;
}

void ArcTool::ShowPropertiesWindow() {
	if (!arcStarted) {
		previewArc.ShowPropertiesWindow();
	}
}

void ArcTool::RenderPreview() {
	if (arcStarted) {
		if (!arcSecondStage) {	// Draw the radius circle
			// Make circle 1 pixel thick
			previewCircle.SetThickness(Navigator::GetInstance()->ConvertScreenToWorkspaceDistance(1));
			previewCircle.RenderPreview();

			// Draw an arrow to indicate the arc direction
			glm::vec2 pos = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(previewCircle.GetCenter());
			glm::vec2 toMouse = Navigator::GetInstance()->mouseSnapped - previewArc.GetCenter();
			float start = angle({ toMouse.x, -toMouse.y });
			glm::vec2 mouse = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(Navigator::GetInstance()->mouseSnapped);
			float rad = glm::distance(pos, mouse) + 20;
			float arcLength = 25;
			float end = start + glm::degrees(arcLength) / rad;
			ApplicationRenderer::DrawArcScreenspace(pos, rad, start, end, 4, { 0, 0, 0, 255 });

			glm::vec2 endPoint = pos + glm::vec2(cos(glm::radians(end)), -sin(glm::radians(end))) * rad;
			float ang2 = start + glm::degrees(arcLength) / rad * 0.7;
			glm::vec2 p1 = pos + glm::vec2(cos(glm::radians(ang2)), -sin(glm::radians(ang2))) * (rad - 5);
			glm::vec2 p2 = pos + glm::vec2(cos(glm::radians(ang2)), -sin(glm::radians(ang2))) * (rad + 5);
			ApplicationRenderer::DrawLineScreenspace(endPoint, p1, 4, { 0, 0, 0, 255 });
			ApplicationRenderer::DrawLineScreenspace(endPoint, p2, 4, { 0, 0, 0, 255 });
		}
		else {					// Draw the actual arc
			previewArc.RenderPreview();
		}
	}
}
