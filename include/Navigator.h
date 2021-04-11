#pragma once

#include "pch.h"
#include "SketchFile.h"
#include "ApplicationRenderer.h"
#include "SelectionHandler.h"
#include "config.h"

#include "Tools/SelectionTool.h"

class Navigator {

	Navigator() {}

	inline static std::unique_ptr<Navigator> instance;

public:
	Navigator(Navigator const&) = delete;
	void operator=(Navigator const&) = delete;

	SketchFile file;
	bool fileChanged = false;
	std::string applicationVersion;
	std::string imguiFileLocation;

	glm::ivec2 windowSize = glm::vec2(0, 0);	// Retrieve once, to be consistent through the update loop
	Battery::ClipboardFormatID clipboardShapeFormat;

	// User interface
	glm::vec2 panOffset = { 0, 0 };
	float scrollFactor = 0.2f;
	float scale = 7;
	float snapSize = 5;
	float mouseHighlightThresholdDistance = 8;
	float currentLineThickness = 0.75;
	bool infiniteSheet = false;
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
	void OnMouseClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel);
	void OnMouseReleased(const glm::vec2& position, bool left, bool right, bool wheel);
	void OnMouseMoved(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy);
	void OnMouseHovered(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy);
	void OnMouseDragged(const glm::vec2& position, const glm::vec2& snapped, float dx, float dy);
	void OnSpaceClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel);
	void OnShapeClicked(const glm::vec2& position, const glm::vec2& snapped, bool left, bool right, bool wheel, ShapeID shape);
	void OnToolChanged();
	void OnLayerSelected(LayerID layer);
	void OnEscapePressed();

	void SelectAll();
	void UndoAction();
	void CopyClipboard();
	void CutClipboard();
	void PasteClipboard();
	bool OpenFile();
	bool OpenEmptyFile();
	bool OpenFile(const std::string& path);
	bool SaveFile();
	bool SaveFileAs();
	void ResetGui();
	bool ExportClipboardRendering(bool transparent = true, float dpi = 300);

	std::string GetMostRecentFile();
	std::vector<std::string> GetRecentFiles();
	bool AppendRecentFile(std::string recentFile);
	bool SaveRecentFiles(std::vector<std::string> recentFiles);
	std::string GetSettingsDirectory();
	std::string GetApplicationVersion();
	void OpenNewWindowFile(const std::string& file);
	void StartNewApplicationInstance();
	void CloseApplication();

	void AddLayer();
	void AddLine(const glm::vec2& p1, const glm::vec2& p2);

	void RenderShapes();
};
