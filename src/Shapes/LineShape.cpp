#pragma once

#include "pch.h"
#include "Shapes/LineShape.h"
#include "ApplicationRenderer.h"
#include "Navigator.h"
#include "Fonts/Fonts.h"

LineShape::LineShape() {

}

LineShape::LineShape(const glm::vec2& p1, const glm::vec2& p2, float thickness, const glm::vec4& color) {
	this->p1 = p1;
	this->p2 = p2;
	this->thickness = thickness;
	this->color = color;
}

LineShape::LineShape(const nlohmann::json& j) {
	LoadJson(j);
}

ShapePTR LineShape::Duplicate() {
	return std::make_shared<LineShape>(p1, p2, thickness, color);
}

std::string LineShape::GetTypeString() const {
	return "Line";
}




std::pair<glm::vec2, glm::vec2> LineShape::GetBoundingBox() const {

	float u = thickness / 2.f;
	glm::vec2 _min = { std::min(p1.x, p2.x) - u, std::min(p1.y, p2.y) - u };
	glm::vec2 _max = { std::max(p1.x, p2.x) + u, std::max(p1.y, p2.y) + u };

	return std::make_pair(_min, _max);
}

bool LineShape::IsInSelectionBox(const glm::vec2& s1, const glm::vec2& s2) const {

	// Should be selected if at least one of the two points is within the selection box
	return (IsInbetween(p1.x, s1.x, s2.x) && IsInbetween(p1.y, s1.y, s2.y)) ||
		(IsInbetween(p2.x, s1.x, s2.x) && IsInbetween(p2.y, s1.y, s2.y));
}

bool LineShape::ShouldBeRendered(float screenWidth, float screenHeight) const {

	auto pair = GetBoundingBox();
	glm::vec2 min = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(pair.first);
	glm::vec2 max = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(pair.second);

	return	!((min.x < 0 && max.x < 0) || (min.x > screenWidth && max.x > screenWidth) ||
			  (min.y < 0 && max.y < 0) || (min.y > screenHeight && max.y > screenHeight));
}

float LineShape::GetDistanceToCursor(const glm::vec2& p) const {
	glm::vec2 aToB = p2 - p1;
	glm::vec2 aToP = p - p1;
	glm::vec2 bToA = p1 - p2;
	glm::vec2 bToP = p - p2;

	if (aToB.length() == 0)
		return (float)aToP.length();

	if (glm::dot(aToB, aToP) < 0) {
		return dist(p1, p);
	}
	else if (glm::dot(bToA, bToP) < 0) {
		return dist(p2, p);
	}
	else {
		return GetNormalDistanceToLine(p);
	}
}

bool LineShape::IsShapeHovered(const glm::vec2& cursor, float thresholdDistance) const {
	return GetDistanceToCursor(cursor) <= thresholdDistance;
}

void LineShape::SetPoint1(const glm::vec2& position) {
	p1 = position;
}

void LineShape::SetPoint2(const glm::vec2& position) {
	p2 = position;
}

void LineShape::SetThickness(float thickness) {
	this->thickness = thickness;
}

void LineShape::SetColor(const glm::vec4& color) {
	this->color = color;
}

glm::vec2 LineShape::GetPoint1() const {
	return p1;
}

glm::vec2 LineShape::GetPoint2() const {
	return p2;
}

float LineShape::GetThickness() const {
	return thickness;
}

glm::vec4 LineShape::GetColor() const {
	return color;
}

glm::vec2 LineShape::GetCenterPosition() const {
	return (p1 + p2) / 2.f;
}

bool LineShape::ShowPropertiesWindow() {
	auto fonts = Battery::__getUserInterface()->GetFontContainer<FontContainer>();
	// Return true if a parameter changed, false if everything stays the same
	ImGui::PushFont(fonts->sansFont22);

	ImGui::Text("Shape type: Line");
	ImGui::Separator();
	ImGui::PushFont(fonts->sansFont17);

	auto oldColor = color;
	auto oldThickness = thickness;

	color /= 255.f;
	ImGui::ColorEdit4("Line color", (float*)&color[0], ImGuiColorEditFlags_NoInputs);
	ImGui::DragFloat("Line thickness", &thickness, 0.1f, 0.f, 10.f);
	color *= 255.f;

	ImGui::PopFont();
	ImGui::PopFont();

	// Something has changed here
	if (oldColor != color || oldThickness != thickness) {
		return true;
	}

	return false;
}

void LineShape::MoveLeft(float amount) {
	p1.x -= amount;
	p2.x -= amount;
}

void LineShape::MoveRight(float amount) {
	p1.x += amount;
	p2.x += amount;
}

void LineShape::MoveUp(float amount) {
	p1.y -= amount;
	p2.y -= amount;
}

void LineShape::MoveDown(float amount) {
	p1.y += amount;
	p2.y += amount;
}

void LineShape::Move(glm::vec2 amount) {
	p1 += amount;
	p2 += amount;
}

void LineShape::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped) {
	p2 = snapped;
}

void LineShape::RenderPreview() const {
	ApplicationRenderer::DrawLineWorkspace(p1, p2, thickness, color);
}

void LineShape::Render(bool layerSelected, bool shapeSelected, bool shapeHovered) const {

	auto& ref = ApplicationRenderer::GetInstance();
	glm::vec4 col = ref.disabledLineColor;

	if (layerSelected) { // Shape is selected
		if (shapeSelected) {
			if (shapeHovered) {	// Shape is selected and hovered
				col = (ref.hoveredLineColor + ref.selectedLineColor) / 2.f;
			}
			else {
				col = ref.selectedLineColor;
			}
		}
		else {			// Shape is simply hovered
			if (shapeHovered) {
				col = ref.hoveredLineColor;
			}
			else {
				col = color;
			}
		}
	}

	ApplicationRenderer::DrawLineWorkspace(p1, p2, thickness, col);

}

void LineShape::RenderExport(glm::vec2 min, glm::vec2 max, float width, float height) const {
	ApplicationRenderer::DrawLineExport(p1, p2, thickness, color, min, max, width, height);
}

nlohmann::json LineShape::GetJson() const {
	nlohmann::json j;

	j["type"] = "line";
	j["p1"] = nlohmann::json::array({p1.x, p1.y});
	j["p2"] = nlohmann::json::array({p2.x, p2.y});
	j["thickness"] = thickness;
	j["color"] = nlohmann::json::array({ color.r, color.g, color.b, color.a });

	return j;
}

bool LineShape::LoadJson(const nlohmann::json& j) {

	try {

		if (j["type"] != "line") {
			return false;
		}

		glm::vec2 p1 = glm::vec2(j["p1"][0], j["p1"][1]);
		glm::vec2 p2 = glm::vec2(j["p2"][0], j["p2"][1]);
		float thickness = j["thickness"];
		glm::vec4 color = glm::vec4(j["color"][0], j["color"][1], j["color"][2], j["color"][3]);

		this->color = color;
		this->thickness = thickness;
		this->p1 = p1;
		this->p2 = p2;

	}
	catch (...) {
		return false;
	}

	return true;
}
