#pragma once

#include "Shapes/GenericShape.h"

class LineShape : public GenericShape {

	glm::vec2 p1 = { 0, 0 };
	glm::vec2 p2 = { 0, 0 };
	float thickness = 0;
	glm::vec4 color;

public:
	LineShape();
	LineShape(const glm::vec2& p1, const glm::vec2& p2, float thickness, const glm::vec4& color);
	LineShape(const nlohmann::json& j);
	ShapePTR Duplicate();
	std::string GetTypeString() const;

	std::pair<glm::vec2, glm::vec2> GetBoundingBox() const;
	bool IsInSelectionBox(const glm::vec2& s1, const glm::vec2& s2) const;
	bool ShouldBeRendered(float screenWidth, float screenHeight) const;

	float GetDistanceToCursor(const glm::vec2& p) const;
	bool IsShapeHovered(const glm::vec2& cursor, float thresholdDistance) const;

	void SetPoint1(const glm::vec2& position);
	void SetPoint2(const glm::vec2& position);
	void SetThickness(float thickness);
	void SetColor(const glm::vec4& color);
	glm::vec2 GetPoint1() const;
	glm::vec2 GetPoint2() const;
	float GetThickness() const;
	glm::vec4 GetColor() const;
	glm::vec2 GetCenterPosition() const;

	bool ShowPropertiesWindow();

	void MoveLeft(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);
	void MoveDown(float amount);
	void Move(glm::vec2 amount);

	void OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped);
	void RenderPreview() const;
	void Render(bool layerSelected, bool shapeSelected, bool shapeHovered) const;
	void RenderExport(glm::vec2 min, glm::vec2 max, float width, float height) const;

	nlohmann::json GetJson() const;
	bool LoadJson(const nlohmann::json& j);




	static inline bool IsInbetween(float v, float v1, float v2) {
		return (v < v1 && v > v2) || (v > v1 && v < v2);
	}

	static inline float dist(const glm::vec2& v) {
		return sqrtf(powf(v.x, 2) + powf(v.y, 2));
	}

	static inline float dist(const glm::vec2& v1, const glm::vec2& v2) {
		return dist(v2 - v1);
	}

	inline float GetNormalDistanceToLine(const glm::vec2& p) const {
		float d = dist(p1, p2);

		if (d == 0)
			return dist(p1, p);

		return abs((p2.y - p1.y) * p.x - (p2.x - p1.x) * p.y + p2.x * p1.y - p2.y * p1.x) / d;
	}

};
