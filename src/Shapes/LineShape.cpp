//#pragma once
//
//#include "battery/graphics/sfml.hpp"
//#include "Shapes/LineShape.h"
//#include "ApplicationRenderer.h"
//#include "Navigator.h"
//#include "Fonts/Fonts.h"
//
//LineShape::LineShape() {
//
//}
//
//LineShape::LineShape(const ImVec2& p1, const ImVec2& p2, float thickness, const ImVec4& color) {
//	this->p1 = p1;
//	this->p2 = p2;
//	this->thickness = thickness;
//	this->color = color;
//}
//
//LineShape::LineShape(const nlohmann::json& j) {
//	LoadJson(j);
//}
//
//ShapePTR LineShape::Duplicate() {
//	return std::make_shared<LineShape>(p1, p2, thickness, color);
//}
//
//std::string LineShape::GetTypeString() const {
//	return "Line";
//}
//
//
//
//
//std::pair<ImVec2, ImVec2> LineShape::GetBoundingBox() const {
//
//	float u = thickness / 2.f;
//	ImVec2 _min = { std::min(p1.x, p2.x) - u, std::min(p1.y, p2.y) - u };
//	ImVec2 _max = { std::max(p1.x, p2.x) + u, std::max(p1.y, p2.y) + u };
//
//	return std::make_pair(_min, _max);
//}
//
//bool LineShape::IsInSelectionBox(const ImVec2& s1, const ImVec2& s2) const {
//
//	// Should be selected if at least one of the two points is within the selection box
//	return (IsInbetween(p1.x, s1.x, s2.x) && IsInbetween(p1.y, s1.y, s2.y)) ||
//		(IsInbetween(p2.x, s1.x, s2.x) && IsInbetween(p2.y, s1.y, s2.y));
//}
//
//bool LineShape::ShouldBeRendered(int screenWidth, int screenHeight) const {
//
//	auto pair = GetBoundingBox();
//	ImVec2 min = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(pair.first);
//	ImVec2 max = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(pair.second);
//
//	return	!((min.x < 0 && max.x < 0) || (min.x > screenWidth && max.x > screenWidth) ||
//			  (min.y < 0 && max.y < 0) || (min.y > screenHeight && max.y > screenHeight));
//}
//
//float LineShape::GetDistanceToCursor(const ImVec2& p) const {
//	ImVec2 aToB = p2 - p1;
//	ImVec2 aToP = p - p1;
//	ImVec2 bToA = p1 - p2;
//	ImVec2 bToP = p - p2;
//
//	if (b::length(aToB) == 0)
//		return b::length(aToP);
//
//	if (b::dot(aToB, aToP) < 0) {
//		return dist(p1, p);
//	}
//	else if (b::dot(bToA, bToP) < 0) {
//		return dist(p2, p);
//	}
//	else {
//		return GetNormalDistanceToLine(p);
//	}
//}
//
//bool LineShape::IsShapeHovered(const ImVec2& cursor, float thresholdDistance) const {
//	return GetDistanceToCursor(cursor) <= thresholdDistance;
//}
//
//void LineShape::SetPoint1(const ImVec2& position) {
//	p1 = position;
//}
//
//void LineShape::SetPoint2(const ImVec2& position) {
//	p2 = position;
//}
//
//void LineShape::SetThickness(float thickness) {
//	this->thickness = thickness;
//}
//
//void LineShape::SetColor(const ImVec4& color) {
//	this->color = color;
//}
//
//ImVec2 LineShape::GetPoint1() const {
//	return p1;
//}
//
//ImVec2 LineShape::GetPoint2() const {
//	return p2;
//}
//
//float LineShape::GetThickness() const {
//	return thickness;
//}
//
//ImVec4 LineShape::GetColor() const {
//	return color;
//}
//
//ImVec2 LineShape::GetCenterPosition() const {
//	return (p1 + p2) / 2.f;
//}
//
//bool LineShape::ShowPropertiesWindow() {
//	// Return true if a parameter changed, false if everything stays the same
//	ImGui::PushFont(Fonts::sansFont22);
//
//	ImGui::Text("Shape type: Line");
//	ImGui::Separator();
//	ImGui::PushFont(Fonts::sansFont17);
//
//	auto oldColor = color;
//	auto oldThickness = thickness;
//
//	color /= 255.f;
//	ImGui::ColorEdit4("Line color", (float*)&color.x, ImGuiColorEditFlags_NoInputs);
//	ImGui::DragFloat("Line thickness", &thickness, 0.1f, 0.f, 10.f);
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
//void LineShape::MoveLeft(float amount) {
//	p1.x -= amount;
//	p2.x -= amount;
//}
//
//void LineShape::MoveRight(float amount) {
//	p1.x += amount;
//	p2.x += amount;
//}
//
//void LineShape::MoveUp(float amount) {
//	p1.y -= amount;
//	p2.y -= amount;
//}
//
//void LineShape::MoveDown(float amount) {
//	p1.y += amount;
//	p2.y += amount;
//}
//
//void LineShape::Move(ImVec2 amount) {
//	p1 += amount;
//	p2 += amount;
//}
//
//void LineShape::OnMouseHovered(const ImVec2& position, const ImVec2& snapped) {
//	p2 = snapped;
//}
//
//void LineShape::RenderPreview() const {
//	ApplicationRenderer::DrawLineWorkspace(p1, p2, thickness, color);
//}
//
//void LineShape::Render(bool layerSelected, bool shapeSelected, bool shapeHovered) const {
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
//	ApplicationRenderer::DrawLineWorkspace(p1, p2, thickness, col);
//
//}
//
//void LineShape::RenderExport(ImVec2 min, ImVec2 max, float width, float height) const {
//	ApplicationRenderer::DrawLineExport(p1, p2, thickness, color, min, max, width, height);
//}
//
//nlohmann::json LineShape::GetJson() const {
//	nlohmann::json j;
//
//	j["type"] = "line";
//	j["p1"] = nlohmann::json::array({p1.x, p1.y});
//	j["p2"] = nlohmann::json::array({p2.x, p2.y});
//	j["thickness"] = thickness;
//	j["color"] = nlohmann::json::array({ color.x, color.y, color.z, color.w });
//
//	return j;
//}
//
//bool LineShape::LoadJson(const nlohmann::json& j) {
//
//	try {
//
//		if (j["type"] != "line") {
//			return false;
//		}
//
//		ImVec2 p1 = ImVec2(j["p1"][0], j["p1"][1]);
//		ImVec2 p2 = ImVec2(j["p2"][0], j["p2"][1]);
//		float thickness = j["thickness"];
//		ImVec4 color = ImVec4(j["color"][0], j["color"][1], j["color"][2], j["color"][3]);
//
//		this->color = color;
//		this->thickness = thickness;
//		this->p1 = p1;
//		this->p2 = p2;
//
//	}
//	catch (...) {
//		return false;
//	}
//
//	return true;
//}
