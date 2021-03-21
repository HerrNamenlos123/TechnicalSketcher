
#include "pch.h"
#include "Shape.h"

int32_t ShapeID::nextID = 0;




static inline bool IsInbetween(float v, float v1, float v2) {
	return (v < v1&& v > v2) || (v > v1 && v < v2);
}

static inline float dist(const glm::vec2& v) {
	return sqrt(pow(v.x, 2) + pow(v.y, 2));
}

static inline float dist(const glm::vec2& v1, const glm::vec2& v2) {
	return dist(v2 - v1);
}






Shape::Shape() {
	type = ShapeType::INVALID;
}

Shape::Shape(enum class ShapeType type, const glm::vec2& p1, const glm::vec2& p2, float thickness, const glm::vec4& color) {
	this->type = type;
	this->p1 = p1;
	this->p2 = p2;
	this->thickness = thickness;
	this->shapeID = ShapeID::MakeID();
	this->color = color;
}

Shape::~Shape() {

}




std::pair<glm::vec2, glm::vec2> Shape::GetBoundingBox() {

	switch (type) {

	case ShapeType::LINE:
	{
		float u = thickness / 2.f;
		glm::vec2 min = { min(p1.x, p2.x) - u, min(p1.y, p2.y) - u };
		glm::vec2 max = { max(p1.x, p2.x) + u, max(p1.y, p2.y) + u };

		return std::make_pair(min, max);
	}

	default:
		break;
	}

	return std::make_pair(glm::vec2(0, 0), glm::vec2(0, 0));
}

bool Shape::IsInSelectionBoundary(const glm::vec2& s1, const glm::vec2& s2) {

	switch (type) {

	case ShapeType::LINE:
	{
		// Should be selected if at least one of the two points is within the selection box
		if ((IsInbetween(p1.x, s1.x, s2.x) && IsInbetween(p1.y, s1.y, s2.y)) || 
			(IsInbetween(p2.x, s1.x, s2.x) && IsInbetween(p2.y, s1.y, s2.y))) {
			return true;
		}
		else {
			return false;
		}
	}

	default:
		break;
	}

	return false;
}

float Shape::GetNormalDistanceToLine(const glm::vec2& p) {
	float d = dist(p1, p2);

	if (d == 0)
		return dist(p1, p);

	return abs((p2.y - p1.y) * p.x - (p2.x - p1.x) * p.y + p2.x * p1.y - p2.y * p1.x) / d;
}

float Shape::GetDistanceToCursorLine(const glm::vec2& p) {
	glm::vec2 aToB = p2 - p1;
	glm::vec2 aToP = p - p1;
	glm::vec2 bToA = p1 - p2;
	glm::vec2 bToP = p - p2;

	if (aToB.length() == 0)
		return aToP.length();

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

float Shape::GetDistanceToCursor(const glm::vec2& cursor) {

	switch (type) {

	case ShapeType::LINE:
	{
		return GetDistanceToCursorLine(cursor);
	}

	default:
		break;
	}

	return 0.f;
}
