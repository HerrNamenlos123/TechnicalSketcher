#pragma once

#include "pch.hpp"

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

	static ShapePTR MakeShape(ShapeType type, ImVec2 p1, ImVec2 p2,
		float thickness, const ImVec4& color);
	static ShapePTR MakeShape(ShapeType type, ImVec2 center,
		float radius, float thickness, const ImVec4& color);
	static ShapePTR MakeShape(ShapeType type, ImVec2 center,
		float radius, float startAngle, float endAngle, float thickness, const ImVec4& color);
	static ShapePTR MakeShape(const nlohmann::json& json);

	virtual ShapePTR Duplicate() = 0;
	virtual std::string GetTypeString() const = 0;

	virtual std::pair<ImVec2, ImVec2> GetBoundingBox() const = 0;
	virtual bool ShouldBeRendered(int screenWidth, int screenHeight) const = 0;
	virtual bool IsInSelectionBox(const ImVec2& s1, const ImVec2& s2) const = 0;
	virtual bool IsShapeHovered(const ImVec2& cursor, float thresholdDistance) const = 0;

	virtual ImVec2 GetCenterPosition() const = 0;

	virtual bool ShowPropertiesWindow() = 0;

	virtual void MoveLeft(float amount) = 0;
	virtual void MoveRight(float amount) = 0;
	virtual void MoveUp(float amount) = 0;
	virtual void MoveDown(float amount) = 0;
	virtual void Move(ImVec2 amount) = 0;

	virtual void OnMouseHovered(const ImVec2& position, const ImVec2& snapped) = 0;

	virtual void RenderPreview() const = 0;
	virtual void Render(bool layerSelected, bool shapeSelected, bool shapeHovered) const = 0;
	virtual void RenderExport(ImVec2 min, ImVec2 max, float width, float height) const = 0;

	virtual nlohmann::json GetJson() const = 0;
	virtual bool LoadJson(const nlohmann::json& j) = 0;
};

