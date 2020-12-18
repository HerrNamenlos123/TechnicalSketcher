
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




nlohmann::json Shape::getJson() {

	// Save all necessary information in a json object

	nlohmann::json json;

	json = nlohmann::json{
		{ "id", shapeID },
		{ "type", type },
		{ "p1", nlohmann::json::array( {p1.x, p1.y} )},
		{ "p2", nlohmann::json::array( {p2.x, p2.y} )},
		{ "thickness", thickness }
	};

	return json;
}
