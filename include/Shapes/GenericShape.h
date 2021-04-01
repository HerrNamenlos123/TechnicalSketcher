#pragma once

#include "pch.h"

typedef size_t ShapeID;

enum class ShapeType {
	NONE,
	LINE
};

class GenericShape {

	ShapeID id = -1;
	inline static ShapeID nextID = 0;

protected:
	GenericShape() {
		id = nextID++;
	}

public:
	ShapeID GetID() {
		return id;
	}

	void SetID(ShapeID id) {
		this->id = id;
	}

	virtual std::unique_ptr<GenericShape> Duplicate() = 0;

	virtual std::pair<glm::vec2, glm::vec2> GetBoundingBox() = 0;
	virtual bool ShouldBeRendered(float screenWidth, float screenHeight) = 0;
	virtual bool IsInSelectionBox(const glm::vec2& s1, const glm::vec2& s2) = 0;
	virtual bool IsShapeHovered(const glm::vec2& cursor, float thresholdDistance) = 0;

	virtual void SetPoint1(const glm::vec2& position) = 0;
	virtual void SetPoint2(const glm::vec2& position) = 0;
	virtual void SetThickness(float thickness) = 0;
	virtual void SetColor(const glm::vec4& color) = 0;
	virtual glm::vec2 GetPoint1() = 0;
	virtual glm::vec2 GetPoint2() = 0;
	virtual float GetThickness() = 0;
	virtual glm::vec4 GetColor() = 0;

	virtual void MoveLeft(float amount) = 0;
	virtual void MoveRight(float amount) = 0;
	virtual void MoveUp(float amount) = 0;
	virtual void MoveDown(float amount) = 0;

	virtual void OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped) = 0;

	virtual void RenderPreview() = 0;
	virtual void Render(bool layerSelected, bool shapeSelected, bool shapeHovered) = 0;

	virtual nlohmann::json GetJson() = 0;
	virtual bool LoadJson(const nlohmann::json& j) = 0;

};

