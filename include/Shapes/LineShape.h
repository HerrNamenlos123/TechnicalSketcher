#pragma once

#include "Shapes/GenericShape.h"
#include "ApplicationRenderer.h"
#include "Navigator.h"

class LineShape : public GenericShape {

	glm::vec2 p1 = { 0, 0 };
	glm::vec2 p2 = { 0, 0 };
	float thickness = 0;
	glm::vec4 color;

public:
	LineShape(const glm::vec2& p1, const glm::vec2& p2, float thickness, const glm::vec4& color);
	LineShape(const nlohmann::json& j);
	std::unique_ptr<GenericShape> Duplicate();

	std::pair<glm::vec2, glm::vec2> GetBoundingBox();
	bool IsInSelectionBox(const glm::vec2& s1, const glm::vec2& s2);
	bool ShouldBeRendered(float screenWidth, float screenHeight);

	float GetDistanceToCursor(const glm::vec2& p);
	bool IsShapeHovered(const glm::vec2& cursor, float thresholdDistance);

	void SetPoint1(const glm::vec2& position);
	void SetPoint2(const glm::vec2& position);
	void SetThickness(float thickness);
	void SetColor(const glm::vec4& color);
	glm::vec2 GetPoint1();
	glm::vec2 GetPoint2();
	float GetThickness();
	glm::vec4 GetColor();

	void MoveLeft(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);
	void MoveDown(float amount);

	void OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped);
	void RenderPreview();
	void Render(bool layerSelected, bool shapeSelected, bool shapeHovered);

	nlohmann::json GetJson();
	bool LoadJson(const nlohmann::json& j);




	static inline bool IsInbetween(float v, float v1, float v2) {
		return (v < v1&& v > v2) || (v > v1 && v < v2);
	}

	static inline float dist(const glm::vec2& v) {
		return sqrt(pow(v.x, 2) + pow(v.y, 2));
	}

	static inline float dist(const glm::vec2& v1, const glm::vec2& v2) {
		return dist(v2 - v1);
	}

	inline float GetNormalDistanceToLine(const glm::vec2& p) {
		float d = dist(p1, p2);

		if (d == 0)
			return dist(p1, p);

		return abs((p2.y - p1.y) * p.x - (p2.x - p1.x) * p.y + p2.x * p1.y - p2.y * p1.x) / d;
	}

};
