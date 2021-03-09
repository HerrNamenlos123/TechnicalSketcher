#pragma once

#include "pch.h"
#include "SketchFile.h"

enum class CursorTool {
	SELECT,
	LINE,
	LINE_STRIP
};

class Navigator : public Battery::Layer {
public:

	SketchFile file;
	Battery::Scene* renderScene = nullptr;

	glm::ivec2 windowSize = glm::vec2(0, 0);	// Retrieve once, to be consistent through the update loop

	// User interface
	glm::vec2 panOffset = { 0, 0 };
	float scrollFactor = -0.2f;
	float scale = 50;
	float snapSize = 1;
	float mouseHighlightThresholdDistance = 8;
	float currentLineThickness = 5;

	// Buffers to store event data
	float scrollBuffer = 0;
	std::vector<Battery::MouseButtonPressedEvent> mousePressedEventBuffer;
	std::vector<Battery::MouseButtonReleasedEvent> mouseReleasedEventBuffer;
	std::vector<Battery::MouseMovedEvent> mouseMovedEventBuffer;

	// Visuals
	float gridLineColor = 220;	// Grayscale
	float gridLineWidth = 1.5;
	//float gridLineFalloff = 0.7;
	glm::vec4 disabledLineColor = glm::vec4(200, 200, 200, 255);
	glm::vec4 normalLineColor = glm::vec4(0, 0, 0, 255);
	glm::vec4 hoveredLineColor = glm::vec4(252, 132, 3, 255);
	glm::vec4 selectedLineColor = glm::vec4(255, 0, 0, 255);
	glm::vec4 selectionBoxOutlineColor = glm::vec4(252, 132, 3, 255);
	glm::vec4 selectionBoxFillColor = glm::vec4(200, 20, 0, 30);

	// Controls
	enum class CursorTool selectedTool = CursorTool::SELECT;
	glm::vec2 mousePosition = { 0, 0 };			// Those mouse positions are in workspace coordinates
	glm::vec2 mouseSnapped = { 0, 0 };

	glm::vec2 previewPointPosition = { 0, 0 };
	bool previewPointShown = false;

	glm::vec2 previewShapePoint1 = { 0, 0 };	// These are temporary for shape previews
	glm::vec2 previewShapePoint2 = { 0, 0 };
	glm::vec2 previewShapePoint3 = { 0, 0 };
	enum class ShapeType previewShape = ShapeType::INVALID;

	glm::vec2 selectionBoxPointA = { 0, 0 };
	glm::vec2 selectionBoxPointB = { 0, 0 };
	bool selectionBoxActive = false;

	std::vector<ShapeID> selectedShapes;
	ShapeID hoveredShape = -1;
	ShapeID lastHoveredShape = -1;

	Navigator();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnEvent(Battery::Event* e) override;


	glm::vec2 ConvertScreenToWorkspaceCoords(const glm::vec2& v);
	glm::vec2 ConvertWorkspaceToScreenCoords(const glm::vec2& v);

	bool IsShapeSelected(ShapeID id);
	void MouseScrolled(float amount);
	void UpdateEvents();
	void UpdateShapes();
	void CancelShape();
	void UseTool(enum class CursorTool tool);

	void OnMouseLeftClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseRightClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseWheelClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseReleased(const glm::vec2& position);
	void OnMouseMoved(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped);
	void OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnToolChanged();

	void AddLine(const glm::vec2& p1, const glm::vec2& p2);

	void DrawGrid();
	void RenderShapes();
	void DrawLine(const glm::vec2& p1, const glm::vec2& p2, float thickness, const glm::vec4 color);
};
