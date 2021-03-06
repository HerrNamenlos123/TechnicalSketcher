
#include "pch.h"
#include "Shape.h"

int32_t ShapeID::nextID = 0;

Shape::Shape() {
	type = ShapeType::INVALID;
}

Shape::Shape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness) {
	this->type = type;
	this->p1 = p1;
	this->p2 = p2;
	this->thickness = thickness;
	this->shapeID = ShapeID::MakeID();
}

Shape::~Shape() {

}
