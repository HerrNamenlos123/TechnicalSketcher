//#pragma once
//
//#include "pch.h"
//#include "Shapes/CircleShape.h"
//#include "Navigator.h"
//#include "Fonts/Fonts.h"
//
//CircleShape::CircleShape() {
//
//}
//
//CircleShape::CircleShape(const ImVec2& center, float radius, float thickness, const ImVec4& color) {
//	this->center = center;
//	this->radius = radius;
//	this->thickness = thickness;
//	this->color = color;
//}
//
//CircleShape::CircleShape(const nlohmann::json& j) {
//	LoadJson(j);
//}
//
//ShapePTR CircleShape::Duplicate() {
//	return std::make_shared<CircleShape>(center, radius, thickness, color);
//}
//
//std::string CircleShape::GetTypeString() const {
//	return "Circle";
//}
//
//
//
//
//std::pair<ImVec2, ImVec2> CircleShape::GetBoundingBox() const {
//
//	float u = abs(thickness) / 2.f;
//	ImVec2 min = center - ImVec2(abs(radius) + u, abs(radius) + u);
//	ImVec2 max = center + ImVec2(abs(radius) + u, abs(radius) + u);
//
//	return std::make_pair(min, max);
//}
//
//bool CircleShape::IsInSelectionBox(const ImVec2& s1, const ImVec2& s2) const {
//
//	// Should be selected if the center of the circle is in the selection
//	return IsInbetween(center.x, s1.x, s2.x) && IsInbetween(center.y, s1.y, s2.y);
//}
//
//bool CircleShape::ShouldBeRendered(int screenWidth, int screenHeight) const {
//
//	auto pair = GetBoundingBox();
//	ImVec2 min = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(pair.first);
//	ImVec2 max = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(pair.second);
//
//	return	!((min.x < 0 && max.x < 0) || (min.x > screenWidth && max.x > screenWidth) ||
//		(min.y < 0 && max.y < 0) || (min.y > screenHeight && max.y > screenHeight));
//}
//
//float CircleShape::GetDistanceToCursor(const ImVec2& p) const {
//	float centerDist = dist(Navigator::GetInstance()->mousePosition, center);
//	return std::min(abs(centerDist - radius), centerDist);
//}
//
//bool CircleShape::IsShapeHovered(const ImVec2& cursor, float thresholdDistance) const {
//	return GetDistanceToCursor(cursor) <= thresholdDistance;
//}
//
//void CircleShape::SetCenter(const ImVec2& position) {
//	this->center = position;
//}
//
//void CircleShape::SetRadius(float radius) {
//	this->radius = radius;
//}
//
//void CircleShape::SetThickness(float thickness) {
//	this->thickness = thickness;
//}
//
//void CircleShape::SetColor(const ImVec4& color) {
//	this->color = color;
//}
//
//ImVec2 CircleShape::GetCenter() const {
//	return center;
//}
//
//float CircleShape::GetRadius() const {
//	return radius;
//}
//
//float CircleShape::GetThickness() const {
//	return thickness;
//}
//
//ImVec4 CircleShape::GetColor() const {
//	return color;
//}
//
//ImVec2 CircleShape::GetCenterPosition() const {
//	return center;
//}
//
//bool CircleShape::ShowPropertiesWindow() {
//	// Return true if a parameter changed, false if everything stays the same
//	ImGui::PushFont(Fonts::sansFont22);
//
//	ImGui::Text("Shape type: Circle");
//	ImGui::Separator();
//	ImGui::PushFont(Fonts::sansFont17);
//
//	auto oldColor = color;
//	auto oldThickness = thickness;
//
//	color /= 255.f;
//	ImGui::ColorEdit4("Circle color", (float*)&color.x, ImGuiColorEditFlags_NoInputs);
//	ImGui::DragFloat("Circle line thickness", &thickness, 0.1f, 0.f, 10.f);
//	color *= 255.f;
//
//	ImGui::PopFont();
//	ImGui::PopFont();
//
//	// Something has changed here
//	if (oldColor != color || oldThickness != thickness) {
//		return true;
//	}
//
//	return false;
//}
//
//void CircleShape::MoveLeft(float amount) {
//	center.x -= amount;
//}
//
//void CircleShape::MoveRight(float amount) {
//	center.x += amount;
//}
//
//void CircleShape::MoveUp(float amount) {
//	center.y -= amount;
//}
//
//void CircleShape::MoveDown(float amount) {
//	center.y += amount;
//}
//
//void CircleShape::Move(ImVec2 amount) {
//	center += amount;
//}
//
//void CircleShape::OnMouseHovered(const ImVec2& position, const ImVec2& snapped) {
//	radius = dist(center, snapped);
//}
//
//void CircleShape::RenderPreview() const {
//	ApplicationRenderer::DrawCircleWorkspace(center, radius, thickness, color);
//}
//
//void CircleShape::Render(bool layerSelected, bool shapeSelected, bool shapeHovered) const {
//
//	auto& ref = ApplicationRenderer::GetInstance();
//	ImVec4 col = ref.disabledLineColor;
//
//	if (layerSelected) { // Shape is selected
//		if (shapeSelected) {
//			if (shapeHovered) {	// Shape is selected and hovered
//				col = (ref.hoveredLineColor + ref.selectedLineColor) / 2.f;
//			}
//			else {
//				col = ref.selectedLineColor;
//			}
//		}
//		else {			// Shape is simply hovered
//			if (shapeHovered) {
//				col = ref.hoveredLineColor;
//			}
//			else {
//				col = color;
//			}
//		}
//	}
//
//	ApplicationRenderer::DrawCircleWorkspace(center, radius, thickness, col);
//}
//
//void CircleShape::RenderExport(ImVec2 min, ImVec2 max, float width, float height) const {
//	ApplicationRenderer::DrawCircleExport(center, radius, thickness, color, min, max, width, height);
//}
//
//nlohmann::json CircleShape::GetJson() const {
//	nlohmann::json j;
//
//	j["type"] = "circle";
//	j["center"] = nlohmann::json::array({ center.x, center.y });
//	j["radius"] = radius;
//	j["thickness"] = thickness;
//	j["color"] = nlohmann::json::array({ color.x, color.y, color.z, color.w });
//
//	return j;
//}
//
//bool CircleShape::LoadJson(const nlohmann::json& j) {
//
//	try {
//
//		if (j["type"] != "circle") {
//			return false;
//		}
//
//		ImVec2 center = ImVec2(j["center"][0], j["center"][1]);
//		float radius = j["radius"];
//		float thickness = j["thickness"];
//		ImVec4 color = ImVec4(j["color"][0], j["color"][1], j["color"][2], j["color"][3]);
//
//		this->color = color;
//		this->thickness = thickness;
//		this->center = center;
//		this->radius = radius;
//
//	}
//	catch (...) {
//		return false;
//	}
//
//	return true;
//}
