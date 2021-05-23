#pragma once

#include "Shapes/GenericShape.h"
#include "ApplicationRenderer.h"

class ArcShape : public GenericShape {

	glm::vec2 center = { 0, 0 };
	float radius = 0;
	float startAngle = 0;
	float endAngle = 0;
	float thickness = 0;
	glm::vec4 color;

public:
	ArcShape();
	ArcShape(const glm::vec2& center, float radius, float angleStart, float angleEnd, float thickness, const glm::vec4& color);
	ArcShape(const nlohmann::json& j);
	ShapePTR Duplicate();
	std::string GetTypeString() const;

	std::pair<glm::vec2, glm::vec2> GetBoundingBox() const;
	bool IsInSelectionBox(const glm::vec2& s1, const glm::vec2& s2) const;
	bool ShouldBeRendered(float screenWidth, float screenHeight) const;

	float GetDistanceToCursor(const glm::vec2& p) const;
	bool IsShapeHovered(const glm::vec2& cursor, float thresholdDistance) const;

	void SetCenter(const glm::vec2& position);
	void SetRadius(float radius);
	void SetStartAngle(float angle);
	void SetEndAngle(float angle);
	void SetThickness(float thickness);
	void SetColor(const glm::vec4& color);
	glm::vec2 GetCenter() const;
	float GetRadius() const;
	float GetStartAngle() const;
	float GetEndAngle() const;
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
		return (v < v1&& v > v2) || (v > v1 && v < v2);
	}

	static inline float dist(const glm::vec2& v) {
		return sqrt(pow(v.x, 2) + pow(v.y, 2));
	}

	static inline float dist(const glm::vec2& v1, const glm::vec2& v2) {
		return dist(v2 - v1);
	}
};
