#pragma once

#include "pch.h"
#include "LayerState.h"

LayerState::LayerState() {
}

LayerState::LayerState(const LayerState& state) {
	for (size_t i = 0; i < state.shapes.size(); i++) {
		shapes.push_back(state.shapes[i]->Duplicate());
	}
}

void LayerState::operator=(const LayerState& state) {
	shapes.clear();
	for (size_t i = 0; i < state.shapes.size(); i++) {
		shapes.push_back(state.shapes[i]->Duplicate());
	}
}

void LayerState::PushShape(std::unique_ptr<GenericShape>&& shape) {
	shapes.push_back(std::move(shape));
}

bool LayerState::RemoveShape(ShapeID id) {
	for (size_t i = 0; i < shapes.size(); i++) {
		if (shapes[i]->GetID() == id) {
			shapes.erase(shapes.begin() + i);
			return true;
		}
	}

	return false;
}

GenericShape* LayerState::FindShape(ShapeID id) {

	for (std::unique_ptr<GenericShape>& shape : shapes) {
		if (shape->GetID() == id) {
			return shape.get();
		}
	}

	return nullptr;
}

const std::vector<std::unique_ptr<GenericShape>>& LayerState::GetShapes() const {
	return shapes;
}
