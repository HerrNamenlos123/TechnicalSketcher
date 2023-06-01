//#pragma once
//
//#include "Shapes/GenericShape.h"
//
//class LineShape : public GenericShape {
//
//	ImVec2 p1 = { 0, 0 };
//	ImVec2 p2 = { 0, 0 };
//	float thickness = 0;
//	ImVec4 color;
//
//public:
//	LineShape();
//	LineShape(const ImVec2& p1, const ImVec2& p2, float thickness, const ImVec4& color);
//	LineShape(const nlohmann::json& j);
//	ShapePTR Duplicate();
//	std::string GetTypeString() const;
//
//	std::pair<ImVec2, ImVec2> GetBoundingBox() const;
//	bool IsInSelectionBox(const ImVec2& s1, const ImVec2& s2) const;
//	bool ShouldBeRendered(int screenWidth, int screenHeight) const;
//
//	float GetDistanceToCursor(const ImVec2& p) const;
//	bool IsShapeHovered(const ImVec2& cursor, float thresholdDistance) const;
//
//	void SetPoint1(const ImVec2& position);
//	void SetPoint2(const ImVec2& position);
//	void SetThickness(float thickness);
//	void SetColor(const ImVec4& color);
//	ImVec2 GetPoint1() const;
//	ImVec2 GetPoint2() const;
//	float GetThickness() const;
//	ImVec4 GetColor() const;
//	ImVec2 GetCenterPosition() const;
//
//	bool ShowPropertiesWindow();
//
//	void MoveLeft(float amount);
//	void MoveRight(float amount);
//	void MoveUp(float amount);
//	void MoveDown(float amount);
//	void Move(ImVec2 amount);
//
//	void OnMouseHovered(const ImVec2& position, const ImVec2& snapped);
//	void RenderPreview() const;
//	void Render(bool layerSelected, bool shapeSelected, bool shapeHovered) const;
//	void RenderExport(ImVec2 min, ImVec2 max, float width, float height) const;
//
//	nlohmann::json GetJson() const;
//	bool LoadJson(const nlohmann::json& j);
//
//
//
//
//	static inline bool IsInbetween(float v, float v1, float v2) {
//		return (v < v1 && v > v2) || (v > v1 && v < v2);
//	}
//
//	static inline float dist(const ImVec2& v) {
//		return sqrtf(powf(v.x, 2) + powf(v.y, 2));
//	}
//
//	static inline float dist(const ImVec2& v1, const ImVec2& v2) {
//		return dist(v2 - v1);
//	}
//
//	inline float GetNormalDistanceToLine(const ImVec2& p) const {
//		float d = dist(p1, p2);
//
//		if (d == 0)
//			return dist(p1, p);
//
//		return abs((p2.y - p1.y) * p.x - (p2.x - p1.x) * p.y + p2.x * p1.y - p2.y * p1.x) / d;
//	}
//
//};
