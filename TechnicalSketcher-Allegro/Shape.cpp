
#include "pch.h"
#include "Shape.h"

Shape::Shape() {
	shapeID = -1;
	type = SHAPE_INVALID;
}

Shape::Shape(ShapeID _shapeID, enum ShapeType _type, glm::vec2 _p1, glm::vec2 _p2, float _thickness) {
	shapeID = _shapeID;
	type = _type;
	p1 = _p1;
	p2 = _p2;
	thickness = _thickness;
}

Shape::~Shape() {

}
