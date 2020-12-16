#pragma once

#include <cstddef>
#include "json.hpp"

typedef size_t ShapeID;

enum ShapeType {
	SHAPE_INVALID,
	SHAPE_LINE,
	SHAPE_CIRCLE
};

struct Shape {

	ShapeID shapeID = -1;
	int type = SHAPE_INVALID;
	glm::vec2 p1 = { 0, 0 };
	glm::vec2 p2 = { 0, 0 };
	float thickness = 0;

	Shape();
	Shape(ShapeID _shapeID, enum ShapeType _type, glm::vec2 _p1, glm::vec2 _p2, float _thickness);
	~Shape();

	nlohmann::json getJson();


};

