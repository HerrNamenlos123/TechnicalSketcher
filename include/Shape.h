#pragma once

#include "pch.h"

// ShapeID gets its own class to make using it wrongly impossible
// -1 is invalid
// The ID gets assigned automatically and is always unique
class ShapeID {

	int32_t id = -1;
	static int32_t nextID;

public:
	ShapeID() {
		id = -1;
	}

	ShapeID(const ShapeID& id) {
		this->id = id.id;
	}

	ShapeID(int32_t id) {
		this->id = id;
	}

	static int32_t MakeID() {
		ShapeID newID(nextID);
		nextID++;
		return newID;
	}

	int32_t Get() const {
		return id;
	}

	bool IsValid() const {
		return id == -1;
	}

	operator int32_t() const {
		return id;
	}

	bool operator==(const ShapeID& id) {
		return this->id == id.id;
	}
};

enum class ShapeType {
	INVALID,
	LINE,
	CIRCLE
};

struct Shape {

	ShapeID shapeID;
	enum class ShapeType type = ShapeType::INVALID;
	glm::vec2 p1 = { 0, 0 };
	glm::vec2 p2 = { 0, 0 };
	float thickness = 0;

	Shape();
	Shape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness);
	~Shape();

};

