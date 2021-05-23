#pragma once

#include "pch.h"

typedef size_t ShapeID;
class GenericShape;
typedef std::shared_ptr<GenericShape> ShapePTR;

enum class ShapeType {
	NONE,
	LINE,
	CIRCLE,
	ARC
};

class GenericShape {

	ShapeID id = -1;
	inline static ShapeID nextID = 0;

protected:
	GenericShape() {
		id = nextID++;
	}

public:
	GenericShape(const GenericShape& shape) = delete;
	void operator=(const GenericShape& shape) = delete;

	ShapeID GetID() const {
		return id;
	}

	void SetID(ShapeID id) {
		this->id = id;
	}

	static ShapePTR MakeShape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2,
		float thickness, const glm::vec4& color);
	static ShapePTR MakeShape(enum class ShapeType type, glm::vec2 center,
		float radius, float thickness, const glm::vec4& color);
	static ShapePTR MakeShape(enum class ShapeType type, glm::vec2 center,
		float radius, float startAngle, float endAngle, float thickness, const glm::vec4& color);
	static ShapePTR MakeShape(const nlohmann::json& json);

	virtual ShapePTR Duplicate() = 0;
	virtual std::string GetTypeString() const = 0;

	virtual std::pair<glm::vec2, glm::vec2> GetBoundingBox() const = 0;
	virtual bool ShouldBeRendered(float screenWidth, float screenHeight) const = 0;
	virtual bool IsInSelectionBox(const glm::vec2& s1, const glm::vec2& s2) const = 0;
	virtual bool IsShapeHovered(const glm::vec2& cursor, float thresholdDistance) const = 0;

	virtual glm::vec2 GetCenterPosition() const = 0;

	virtual bool ShowPropertiesWindow() = 0;

	virtual void MoveLeft(float amount) = 0;
	virtual void MoveRight(float amount) = 0;
	virtual void MoveUp(float amount) = 0;
	virtual void MoveDown(float amount) = 0;
	virtual void Move(glm::vec2 amount) = 0;

	virtual void OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped) = 0;

	virtual void RenderPreview() const = 0;
	virtual void Render(bool layerSelected, bool shapeSelected, bool shapeHovered) const = 0;
	virtual void RenderExport(glm::vec2 min, glm::vec2 max, float width, float height) const = 0;

	virtual nlohmann::json GetJson() const = 0;
	virtual bool LoadJson(const nlohmann::json& j) = 0;
};

