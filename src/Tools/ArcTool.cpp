//
//#include "pch.h"
//#include "Tools/ArcTool.h"
//#include "Navigator.h"
//
//static float mag(const ImVec2& v) {
//	return sqrt(pow(v.x, 2) + pow(v.y, 2));
//}
//
////
//// double angle(const ImVec2& v);
////
//// Angle from X-Axis to vector, range 0 to 360 counterclockwise
////
////       Y
////       |     P
////       |    /
////       |   /
////       |  / \.
////       | / a |
////       |------------------X
////
//static float angle(const ImVec2& v) {
//	if (mag(v) > 0) {
//		if (v.y >= 0) {
//			return b::degrees(acos(v.x / mag(v)));
//		}
//		else if (v.y <= 0) {
//			return 360.f - b::degrees(acos(v.x / mag(v)));
//		}
//	}
//
//	return 0;
//}
//
//
//
//
//void ArcTool::OnToolChanged() {
//	Navigator::GetInstance()->previewPointShown = true;
//	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
//}
//
//void ArcTool::OnSpaceClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel) {
//	if (left) {
//		if (!arcStarted) {			// Start the arc
//			previewArc.SetCenter(snapped);
//			previewArc.SetRadius(0);
//			previewCircle.SetCenter(snapped);
//			previewArc.SetRadius(0);
//			previewCircle.SetThickness(1);
//			previewCircle.SetColor({ 0, 0, 0, 255 });
//			Navigator::GetInstance()->previewPointShown = true;
//			arcStarted = true;
//			arcSecondStage = false;
//		}
//		else {						// Continue the arc
//			if (!arcSecondStage) {
//				arcSecondStage = true;
//				previewArc.SetRadius(b::distance(snapped, previewArc.GetCenter()));
//
//				ImVec2 toMouse = Navigator::GetInstance()->mouseSnapped - previewArc.GetCenter();
//				toMouse.y *= -1;
//				previewArc.SetStartAngle(angle(toMouse));
//			}
//			else {				// Finish the arc
//				Navigator::GetInstance()->AddArc(previewArc);
//				CancelShape();
//			}
//		}
//	}
//	else {
//		CancelShape();
//	}
//}
//
//void ArcTool::OnShapeClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel, ShapeID shape) {
//
//}
//
//void ArcTool::OnMouseHovered(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
//	if (arcStarted) {
//		if (!arcSecondStage) {	// First stage
//			previewCircle.SetRadius(b::distance(snapped, previewArc.GetCenter()));
//		}
//		else {					// Second stage
//			ImVec2 toMouse = Navigator::GetInstance()->mouseSnapped - previewArc.GetCenter();
//			toMouse.y *= -1;
//			previewArc.SetEndAngle(angle(toMouse));
//		}
//	}
//
//	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
//}
//
//void ArcTool::OnMouseDragged(const ImVec2& position, const ImVec2& snapped, int dx, int dy) {
//
//}
//
//void ArcTool::OnMouseReleased(const ImVec2& position, bool left, bool right, bool wheel) {
//
//}
//
//void ArcTool::OnLayerSelected(LayerID layer) {
//
//}
//
//void ArcTool::CancelShape() {
//	Navigator::GetInstance()->previewPointPosition = Navigator::GetInstance()->mouseSnapped;
//	Navigator::GetInstance()->previewPointShown = true;
//	arcStarted = false;
//	arcSecondStage = false;
//}
//
//void ArcTool::SelectAll() {
//
//}
//
//void ArcTool::CopyClipboard() {
//
//}
//
//void ArcTool::CutClipboard() {
//
//}
//
//void ArcTool::PasteClipboard() {
//
//}
//
//bool ArcTool::StepToolBack() {
//	if (arcStarted) {
//		CancelShape();
//		return true;
//	}
//
//	return false;
//}
//
//bool ArcTool::IsPropertiesWindowShown() {
//	if (!arcStarted) {
//		return true;
//	}
//
//	return false;
//}
//
//void ArcTool::ShowPropertiesWindow() {
//	if (!arcStarted) {
//		previewArc.ShowPropertiesWindow();
//	}
//}
//
//void ArcTool::RenderPreview() {
//	if (arcStarted) {
//		if (!arcSecondStage) {	// Draw the radius circle
//			// Make circle 1 pixel thick
//			previewCircle.SetThickness(Navigator::GetInstance()->ConvertScreenToWorkspaceDistance(1));
//			previewCircle.RenderPreview();
//
//			// Draw an arrow to indicate the arc direction
//			ImVec2 pos = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(previewCircle.GetCenter());
//			ImVec2 toMouse = Navigator::GetInstance()->mouseSnapped - previewArc.GetCenter();
//			float start = angle({ toMouse.x, -toMouse.y });
//			ImVec2 mouse = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(Navigator::GetInstance()->mouseSnapped);
//			float rad = b::distance(pos, mouse) + 20;
//			float arcLength = 25;
//			float end = start + b::degrees(arcLength) / rad;
//			ApplicationRenderer::DrawArcScreenspace(pos, rad, start, end, 4, { 0, 0, 0, 255 });
//
//			ImVec2 endPoint = pos + ImVec2(cos(b::radians(end)), -sin(b::radians(end))) * rad;
//			float ang2 = start + b::degrees(arcLength) / rad * 0.7;
//			ImVec2 p1 = pos + ImVec2(cos(b::radians(ang2)), -sin(b::radians(ang2))) * (rad - 5);
//			ImVec2 p2 = pos + ImVec2(cos(b::radians(ang2)), -sin(b::radians(ang2))) * (rad + 5);
//			ApplicationRenderer::DrawLineScreenspace(endPoint, p1, 4, { 0, 0, 0, 255 });
//			ApplicationRenderer::DrawLineScreenspace(endPoint, p2, 4, { 0, 0, 0, 255 });
//		}
//		else {					// Draw the actual arc
//			previewArc.RenderPreview();
//		}
//	}
//}
