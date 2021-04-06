
#include "pch.h"
#include "Shapes/GenericShape.h"
#include "Shapes/LineShape.h"

ShapePTR GenericShape::MakeShape(enum class ShapeType type, glm::vec2 p1,
	glm::vec2 p2, float thickness, const glm::vec4& color) {

	switch (type) {

	case ShapeType::NONE:
		LOG_WARN(__FUNCTION__"(): Can't add shape of type 'NONE'");
		return std::unique_ptr<GenericShape>();

	case ShapeType::LINE:
		return std::make_unique<LineShape>(p1, p2, thickness, color);

	default:
		LOG_WARN(__FUNCTION__"(): Unsupported shape type");
		return std::unique_ptr<GenericShape>();
	}
}

ShapePTR GenericShape::MakeShape(const nlohmann::json& json) {

	try {
		std::string type = json["type"];

		if (type == "line") {
			auto shape = std::make_unique<LineShape>();

			if (shape->LoadJson(json)) {
				return shape;
			}
		}
		else {
			LOG_ERROR("Can't parse shape from JSON: Invalid shape type!");
			return std::unique_ptr<GenericShape>();
		}
	}
	catch (...) {}

	LOG_ERROR("Can't parse shape from JSON: Invalid JSON format!");
	return std::unique_ptr<GenericShape>();
}