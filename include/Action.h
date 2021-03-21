#pragma once

#include "Shape.h"

enum class ActionType {
	NONE,
	SHAPE_ADD,
	SHAPE_REMOVE,
	SHAPE_MOVE_LEFT,
	SHAPE_MOVE_RIGHT,
	SHAPE_MOVE_UP,
	SHAPE_MOVE_DOWN
};

class Action {
public:

	enum class ActionType actionType = ActionType::NONE;
	Shape shape;
	float amountMoved = 0;

	Action(enum class ActionType actionType, const Shape& shape, float amountMoved = 0) {

		this->actionType = actionType;
		
		switch (actionType) {

		case ActionType::SHAPE_ADD:
			this->shape = shape;
			break;

		case ActionType::SHAPE_REMOVE:
			this->shape = shape;
			break;

		case ActionType::SHAPE_MOVE_LEFT:
		case ActionType::SHAPE_MOVE_RIGHT:
		case ActionType::SHAPE_MOVE_UP:
		case ActionType::SHAPE_MOVE_DOWN:
			this->shape = shape;
			this->amountMoved = amountMoved;
			break;

		default:
			break;
		}
	}
};
