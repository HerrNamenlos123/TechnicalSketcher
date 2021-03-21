#pragma once

#include "pch.h"
#include "Shape.h"
#include "config.h"
#include "Action.h"

// LayerID gets its own class to make using it wrongly impossible
// -1 is invalid
// The ID gets assigned automatically and is always unique
class LayerID {

	int32_t id = -1;
	static int32_t nextID;

public:
	LayerID() {
		id = -1;
	}

	LayerID(const LayerID& id) {
		this->id = id.id;
	}

	LayerID(int32_t id) {
		this->id = id;
	}

	static int32_t MakeID() {
		LayerID newID(nextID);
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

	bool operator==(const LayerID& id) {
		return this->id == id.id;
	}
};

class Layer {

	std::vector<Shape> shapes;

public:
	std::string name;
	LayerID layerID;
	Battery::Texture2D previewImage;
	std::vector<Action> actions;

	Layer(const std::string& name) {
		this->name = name;
		layerID = LayerID::MakeID();
	}
	
	void AddShape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness, const glm::vec4& color) {
		AddAction(Action(ActionType::SHAPE_ADD, Shape(type, p1, p2, thickness, color)));
	}
	
	bool RemoveShape(const ShapeID& id, bool noAction = false) {

		Shape* shape = FindShape(id);

		if (shape == nullptr)
			return false;

		if (!noAction)
			AddAction(Action(ActionType::SHAPE_REMOVE, *shape));

		return true;
	}

	bool MoveShapeLeft(const ShapeID& id, float amount) {

		Shape* shape = FindShape(id);

		if (shape == nullptr)
			return false;

		AddAction(Action(ActionType::SHAPE_MOVE_LEFT, *shape, amount));
		return true;
	}

	bool MoveShapeRight(const ShapeID& id, float amount) {

		Shape* shape = FindShape(id);

		if (shape == nullptr)
			return false;

		AddAction(Action(ActionType::SHAPE_MOVE_RIGHT, *shape, amount));
		return true;
	}

	bool MoveShapeUp(const ShapeID& id, float amount) {

		Shape* shape = FindShape(id);

		if (shape == nullptr)
			return false;

		AddAction(Action(ActionType::SHAPE_MOVE_UP, *shape, amount));
		return true;
	}

	bool MoveShapeDown(const ShapeID& id, float amount) {

		Shape* shape = FindShape(id);

		if (shape == nullptr)
			return false;

		AddAction(Action(ActionType::SHAPE_MOVE_DOWN, *shape, amount));
		return true;
	}

	void AddAction(Action action) {

		while (actions.size() >= MAX_NUMBER_OF_UNDOS) {
			actions.erase(actions.begin());
		}

		actions.push_back(action);
		ApplyAction(&action);
	}

	void ApplyAction(Action* action) {

		Shape* actionShape = &action->shape;
		Shape* shapeReference = FindShape(action->shape.shapeID);

		switch (action->actionType) {

		case ActionType::SHAPE_ADD:
		{
			LOG_TRACE("Shape added to layer {} with id ", actionShape->shapeID.Get());
			shapes.push_back(*actionShape);
		}
			break;

		case ActionType::SHAPE_REMOVE:
		{
			if (shapeReference == nullptr) {
				LOG_TRACE("Can't remove Shape #{}: Not found in Layer '{}'", shapeReference->shapeID.Get(), name);
				return;
			}

			for (size_t i = 0; i < shapes.size(); i++) {
				if (shapes[i].shapeID == shapeReference->shapeID) {
					shapes.erase(shapes.begin() + i);
					return;
				}
			}
		}
		break;

		case ActionType::SHAPE_MOVE_LEFT:
			if (shapeReference == nullptr) {
				LOG_CORE_TRACE(__FUNCTION__"(): Can't apply move action: Shape can't be found!");
				return;
			}
			shapeReference->p1.x -= action->amountMoved;
			shapeReference->p2.x -= action->amountMoved;
			break;

		case ActionType::SHAPE_MOVE_RIGHT:
			if (shapeReference == nullptr) {
				LOG_CORE_TRACE(__FUNCTION__"(): Can't apply move action: Shape can't be found!");
				return;
			}
			shapeReference->p1.x += action->amountMoved;
			shapeReference->p2.x += action->amountMoved;
			break;

		case ActionType::SHAPE_MOVE_UP:
			if (shapeReference == nullptr) {
				LOG_CORE_TRACE(__FUNCTION__"(): Can't apply move action: Shape can't be found!");
				return;
			}
			shapeReference->p1.y -= action->amountMoved;
			shapeReference->p2.y -= action->amountMoved;
			break;

		case ActionType::SHAPE_MOVE_DOWN:
			if (shapeReference == nullptr) {
				LOG_CORE_TRACE(__FUNCTION__"(): Can't apply move action: Shape can't be found!");
				return;
			}
			shapeReference->p1.y += action->amountMoved;
			shapeReference->p2.y += action->amountMoved;
			break;

		default:
			break;
		}

	}

	void UndoAction(Action* action) {

		Shape* actionShape = &action->shape;
		Shape* shapeReference = FindShape(action->shape.shapeID);

		LOG_WARN("ID: {}", action->shape.shapeID.Get());

		switch (action->actionType) {

		case ActionType::SHAPE_ADD:
		{
			if (shapeReference == nullptr) {
				LOG_WARN("Can't remove Shape #{}: Not found in Layer '{}'", actionShape->shapeID.Get(), name);
				return;
			}
			LOG_TRACE(__FUNCTION__"(): Undoing add action: Removing shape #{}", shapeReference->shapeID.Get());

			for (size_t i = 0; i < shapes.size(); i++) {
				if (shapes[i].shapeID == shapeReference->shapeID) {
					shapes.erase(shapes.begin() + i);
					return;
				}
			}
		}
		break;

		case ActionType::SHAPE_REMOVE:
		{
			LOG_TRACE(__FUNCTION__"(): Undoing remove action: Readding shape #{}", actionShape->shapeID.Get());
			shapes.push_back(*actionShape);
		}
		break;

		case ActionType::SHAPE_MOVE_LEFT:
			if (shapeReference == nullptr) {
				LOG_CORE_TRACE(__FUNCTION__"(): Can't undo move action: Shape can't be found!");
				return;
			}
			shapeReference->p1.x += action->amountMoved;
			shapeReference->p2.x += action->amountMoved;
			break;

		case ActionType::SHAPE_MOVE_RIGHT:
			if (shapeReference == nullptr) {
				LOG_CORE_TRACE(__FUNCTION__"(): Can't undo move action: Shape can't be found!");
				return;
			}
			shapeReference->p1.x -= action->amountMoved;
			shapeReference->p2.x -= action->amountMoved;
			break;

		case ActionType::SHAPE_MOVE_UP:
			if (shapeReference == nullptr) {
				LOG_CORE_TRACE(__FUNCTION__"(): Can't undo move action: Shape can't be found!");
				return;
			}
			shapeReference->p1.y += action->amountMoved;
			shapeReference->p2.y += action->amountMoved;
			break;

		case ActionType::SHAPE_MOVE_DOWN:
			if (shapeReference == nullptr) {
				LOG_CORE_TRACE(__FUNCTION__"(): Can't undo move action: Shape can't be found!");
				return;
			}
			shapeReference->p1.y -= action->amountMoved;
			shapeReference->p2.y -= action->amountMoved;
			break;

		default:
			break;
		}

	}

	void UndoPreviousAction() {

		if (actions.size() >= 1) {
			Action* action = &actions[actions.size() - 1];
			UndoAction(action);
			LOG_ERROR("Undoing action {}", action->shape.shapeID.Get());
			actions.pop_back();

			LOG_WARN("Actions: ");
			for (size_t i = 0; i < actions.size(); i++) {
				LOG_WARN("Action shape ID: {}", actions[i].shape.shapeID.Get());
			}
		}
		else {
			LOG_WARN(__FUNCTION__"(): Action not undone: No action was queued");
		}
	}

	void SetPreviewImage(const Battery::Texture2D& image) {
		previewImage = image;
	}
	
	std::vector<Shape>& GetShapes() {
		return shapes;
	}

	size_t Size() {
		return shapes.size();
	}

	Shape* FindShape(const ShapeID& shape) {

		for (size_t i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == shape) {
				return &shapes[i];
			}
		}

		return nullptr;
	}

	nlohmann::json GetJson() {
		return nlohmann::json();
	}
};