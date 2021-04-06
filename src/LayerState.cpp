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

void LayerState::PushShape(ShapePTR&& shape) {
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

std::optional<std::reference_wrapper<GenericShape>> LayerState::FindShape(ShapeID id) {

	for (auto& shape : shapes) {
		if (shape->GetID() == id) {
			return std::make_optional<std::reference_wrapper<GenericShape>>(*shape);
		}
	}

	return std::nullopt;
}

bool LayerState::ShapeExists(const ShapeID& id) const {

	for (auto& shape : shapes) {
		if (shape->GetID() == id) {
			return true;
		}
	}

	return false;
}

const std::vector<ShapePTR>& LayerState::GetShapes() const {
	return shapes;
}

bool LayerState::LoadJson(nlohmann::json json) {
	try {
		// Store all shapes temporarily
		std::vector<ShapePTR> tempShapes;
		for (nlohmann::json shapeJson : json) {
			ShapePTR shape = GenericShape::MakeShape(shapeJson);

			if (!shape) {
				return false;
			}

			tempShapes.push_back(std::move(shape));
		}

		// Now apply all shapes
		shapes.clear();
		for (ShapePTR& shape : tempShapes) {
			shapes.push_back(std::move(shape));
		}
		return true;
	}
	catch (...) {
		LOG_ERROR(__FUNCTION__"(): Can't parse JSON: Invalid format!");
	}

	return false;
}

nlohmann::json LayerState::GetJson() {
	nlohmann::json json = nlohmann::json::array();

	for (ShapePTR& shape : shapes) {
		json.push_back(shape->GetJson());
	}

	return json;
}
