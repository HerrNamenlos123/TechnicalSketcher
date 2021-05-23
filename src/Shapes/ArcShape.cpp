#pragma once

#include "pch.h"
#include "Shapes/ArcShape.h"
#include "Navigator.h"

extern ImFont* materialFont35;
extern ImFont* segoeFont35;
extern ImFont* materialFont22;
extern ImFont* segoeFont22;
extern ImFont* sansFont22;
extern ImFont* sansFont17;
extern ImFont* sansFont9;


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
/*
float arcCircleDistance(glm::vec2 P, glm::vec2 center, float radius) {
	float distanceToCenter = glm::distance(P, center);
	return abs(radius - distanceToCenter);
}

float distanceAroundArc(glm::vec2 P, glm::vec2 center, float radius, float startAngle, float endAngle) {
	float a = angle(P - center);

	startAngle = fmod(startAngle, 360);
	endAngle = fmod(endAngle, 360);

	if (startAngle < endAngle) {
		if (a > startAngle && a < endAngle) {
			return arcCircleDistance(P, center, radius);
		}
	}
	else {
		if (a < startAngle && a > endAngle) {
			//return arcCircleDistance(P, center, radius);
		}
	}
	return 100;

	glm::vec2 p1 = center + glm::vec2(cos(startAngle), -sin(startAngle)) * radius;
	glm::vec2 p2 = center + glm::vec2(cos(endAngle), -sin(endAngle)) * radius;

	float distance1 = distance(P, p1);
	float distance2 = distance(P, p2);

	return min(distance1, distance2);
}*/




ArcShape::ArcShape() {

}

ArcShape::ArcShape(const glm::vec2& center, float radius, float startAngle, float endAngle, float thickness, const glm::vec4& color) {
	this->center = center;
	this->radius = radius;
	this->startAngle = startAngle;
	this->endAngle = endAngle;
	this->thickness = thickness;
	this->color = color;
}

ArcShape::ArcShape(const nlohmann::json& j) {
	LoadJson(j);
}

ShapePTR ArcShape::Duplicate() {
	return std::make_shared<ArcShape>(center, radius, startAngle, endAngle, thickness, color);
}

std::string ArcShape::GetTypeString() const {
	return "Arc";
}




std::pair<glm::vec2, glm::vec2> ArcShape::GetBoundingBox() const {

	float u = abs(thickness) / 2.f;
	glm::vec2 min = center - glm::vec2(abs(radius) + u, abs(radius) + u);
	glm::vec2 max = center + glm::vec2(abs(radius) + u, abs(radius) + u);

	return std::make_pair(min, max);
}

bool ArcShape::IsInSelectionBox(const glm::vec2& s1, const glm::vec2& s2) const {

	// Should be selected if the center of the arc is in the selection
	return IsInbetween(center.x, s1.x, s2.x) && IsInbetween(center.y, s1.y, s2.y);
}

bool ArcShape::ShouldBeRendered(float screenWidth, float screenHeight) const {

	auto pair = GetBoundingBox();
	glm::vec2 min = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(pair.first);
	glm::vec2 max = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(pair.second);

	return	!((min.x < 0 && max.x < 0) || (min.x > screenWidth && max.x > screenWidth) ||
		      (min.y < 0 && max.y < 0) || (min.y > screenHeight && max.y > screenHeight));
}

float ArcShape::GetDistanceToCursor(const glm::vec2& p) const {
	float centerDist = dist(Navigator::GetInstance()->mousePosition, center);
	float arcDist = abs(centerDist - radius);
	return min(arcDist, centerDist);
}

bool ArcShape::IsShapeHovered(const glm::vec2& cursor, float thresholdDistance) const {
	return GetDistanceToCursor(cursor) <= thresholdDistance;
}

void ArcShape::SetCenter(const glm::vec2& position) {
	this->center = position;
}

void ArcShape::SetRadius(float radius) {
	this->radius = radius;
}

void ArcShape::SetStartAngle(float angle) {
	this->startAngle = angle;
}

void ArcShape::SetEndAngle(float angle) {
	this->endAngle = angle;
}

void ArcShape::SetThickness(float thickness) {
	this->thickness = thickness;
}

void ArcShape::SetColor(const glm::vec4& color) {
	this->color = color;
}

glm::vec2 ArcShape::GetCenter() const {
	return center;
}

float ArcShape::GetRadius() const {
	return radius;
}

float ArcShape::GetStartAngle() const {
	return startAngle;
}

float ArcShape::GetEndAngle() const {
	return endAngle;
}

float ArcShape::GetThickness() const {
	return thickness;
}

glm::vec4 ArcShape::GetColor() const {
	return color;
}

glm::vec2 ArcShape::GetCenterPosition() const {
	return center;
}

bool ArcShape::ShowPropertiesWindow() {
	// Return true if a parameter changed, false if everything stays the same
	ImGui::PushFont(sansFont22);

	ImGui::Text("Shape type: Arc");
	ImGui::Separator();
	ImGui::PushFont(sansFont17);

	auto oldColor = color;
	auto oldThickness = thickness;

	color /= 255;
	ImGui::ColorEdit4("Arc color", (float*)&color[0], ImGuiColorEditFlags_NoInputs);
	ImGui::DragFloat("Arc line thickness", &thickness, 0.1f, 0.f, 10.f);
	color *= 255;

	ImGui::PopFont();
	ImGui::PopFont();

	// Something has changed here
	if (oldColor != color || oldThickness != thickness) {
		return true;
	}

	return false;
}

void ArcShape::MoveLeft(float amount) {
	center.x -= amount;
}

void ArcShape::MoveRight(float amount) {
	center.x += amount;
}

void ArcShape::MoveUp(float amount) {
	center.y -= amount;
}

void ArcShape::MoveDown(float amount) {
	center.y += amount;
}

void ArcShape::Move(glm::vec2 amount) {
	center += amount;
}

void ArcShape::OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped) {
	radius = dist(center, snapped);
}

void ArcShape::RenderPreview() const {
	ApplicationRenderer::DrawArcWorkspace(center, radius, startAngle, endAngle, thickness, color);
}

void ArcShape::Render(bool layerSelected, bool shapeSelected, bool shapeHovered) const {

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

	ApplicationRenderer::DrawArcWorkspace(center, radius, startAngle, endAngle, thickness, col);
}

void ArcShape::RenderExport(glm::vec2 min, glm::vec2 max, float width, float height) const {
	ApplicationRenderer::DrawArcExport(center, radius, startAngle, endAngle, thickness, color, min, max, width, height);
}

nlohmann::json ArcShape::GetJson() const {
	nlohmann::json j;

	j["type"] = "arc";
	j["center"] = nlohmann::json::array({ center.x, center.y });
	j["radius"] = radius;
	j["start_angle"] = startAngle;
	j["end_angle"] = endAngle;
	j["thickness"] = thickness;
	j["color"] = nlohmann::json::array({ color.r, color.g, color.b, color.a });

	return j;
}

bool ArcShape::LoadJson(const nlohmann::json& j) {

	try {

		if (j["type"] != "arc") {
			return false;
		}

		glm::vec2 center = glm::vec2(j["center"][0], j["center"][1]);
		float radius = j["radius"];
		float startAngle = j["start_angle"];
		float endAngle = j["end_angle"];
		float thickness = j["thickness"];
		glm::vec4 color = glm::vec4(j["color"][0], j["color"][1], j["color"][2], j["color"][3]);

		this->color = color;
		this->thickness = thickness;
		this->center = center;
		this->radius = radius;
		this->startAngle = startAngle;
		this->endAngle = endAngle;

	}
	catch (...) {
		return false;
	}

	return true;
}
