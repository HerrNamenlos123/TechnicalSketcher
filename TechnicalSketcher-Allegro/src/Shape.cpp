
#include "pch.h"
#include "Shape.h"

Shape::Shape() {
	shapeID = -1;
	type = SHAPE_INVALID;
}

Shape::Shape(nlohmann::json j) {

	shapeID = j["id"];
	type = j["type"];

	std::vector<float> _p1 = j["p1"];
	p1.x = _p1[0];
	p1.y = _p1[1];

	std::vector<float> _p2 = j["p2"];
	p2.x = _p2[0];
	p2.y = _p2[1];

	thickness = j["thickness"];
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
