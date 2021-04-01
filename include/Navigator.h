#pragma once

#include "pch.h"
#include "SketchFile.h"
#include "ApplicationRenderer.h"
#include "SelectionHandler.h"

#include "Tools/SelectionTool.h"

class Navigator {

	Navigator() {}

	inline static std::unique_ptr<Navigator> instance;

public:
	Navigator(Navigator const&) = delete;
	void operator=(Navigator const&) = delete;

	SketchFile file;

	glm::ivec2 windowSize = glm::vec2(0, 0);	// Retrieve once, to be consistent through the update loop

	// User interface
	glm::vec2 panOffset = { 0, 0 };
	float scrollFactor = 0.2f;
	float scale = 50;
	float snapSize = 1;
	float mouseHighlightThresholdDistance = 8;
	float currentLineThickness = 0.1;
	glm::vec4 currentShapeColor = glm::vec4(0, 0, 0, 255);

	// Buffers to store event data
	float scrollBuffer = 0;
	std::vector<Battery::MouseButtonPressedEvent> mousePressedEventBuffer;
	std::vector<Battery::MouseButtonReleasedEvent> mouseReleasedEventBuffer;
	std::vector<Battery::MouseMovedEvent> mouseMovedEventBuffer;
	std::vector<Battery::KeyPressedEvent> keyPressedEventBuffer;

	// Controls
	glm::vec2 mousePosition = { 0, 0 };			// Those mouse positions are in workspace coordinates
	glm::vec2 mouseSnapped = { 0, 0 };
	bool controlKeyPressed = false;
	bool shiftKeyPressed = false;

	glm::vec2 previewPointPosition = { 0, 0 };
	bool previewPointShown = false;

	std::unique_ptr<GenericTool> selectedTool = std::make_unique<SelectionTool>();

	static void CreateInstance();
	static void DestroyInstance();
	static Navigator* GetInstance();

	void OnAttach();
	void OnDetach();
	void OnUpdate();
	void OnRender();
	void OnEvent(Battery::Event* e);


	glm::vec2 ConvertScreenToWorkspaceCoords(const glm::vec2& v);
	glm::vec2 ConvertWorkspaceToScreenCoords(const glm::vec2& v);
	float ConvertWorkspaceToScreenDistance(float distance);
	float ConvertScreenToWorkspaceDistance(float distance);

	void MouseScrolled(float amount);
	void UpdateEvents();
	void CancelShape();
	void UseTool(enum class ToolType tool);
	void PrintShapes();
	void RemoveSelectedShapes();
	void MoveSelectedShapesLeft();
	void MoveSelectedShapesRight();
	void MoveSelectedShapesUp();
	void MoveSelectedShapesDown();
	void SelectNextPossibleShape();

	void OnKeyPressed(Battery::KeyPressedEvent* event);
	void OnMouseLeftClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseRightClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseWheelClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseReleased(const glm::vec2& position);
	void OnMouseMoved(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped);
	void OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped);
	void OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped);
	void OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, ShapeID shape);
	void OnToolChanged();
	void OnLayerSelected(LayerID layer);

	void SelectAll();
	void Print();
	void CopyClipboard();
	void CutClipboard();
	void PasteClipboard();
	void OpenFile();
	void SaveFile();
	void SaveFileAs();

	void AddLayer();
	void AddLine(const glm::vec2& p1, const glm::vec2& p2);

	void RenderShapes();
};
