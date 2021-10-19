#pragma once

#include "pch.h"
#include "SketchFile.h"
#include "ApplicationRenderer.h"
#include "SelectionHandler.h"
#include "config.h"

#include "Tools/SelectionTool.h"
#include "Tools/LineTool.h"
#include "Tools/LineStripTool.h"
#include "Tools/CircleTool.h"
#include "Tools/ArcTool.h"

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

	glm::ivec2 windowSize = glm::vec2(0, 0);	// Retrieve every frame, to be consistent through the update loop
	Battery::ClipboardFormatID clipboardShapeFormat;

	// User interface
	glm::vec2 panOffset = { 0, 0 };
	float scrollFactor = 0.2f;
	float scale = 7;
	float defaultSnapSize = 5;
	float snapSize = defaultSnapSize;
	float mouseHighlightThresholdDistance = 8;
	bool infiniteSheet = false;
	bool gridShown = true;

	float exportDPI = 300;
	bool exportTransparent = true;
	bool popupExportOpen = false;
	bool popupDeleteLayerOpen = false;
	bool popupSettingsOpen = false;
	bool keepUpToDate = true;
	bool tabbedShapeInfo = false;

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

	SelectionTool selectionTool;
	LineTool lineTool;
	LineStripTool lineStripTool;
	CircleTool circleTool;
	ArcTool arcTool;

	GenericTool* selectedTool = &selectionTool;		// This is a reference to the stack objects above, don't delete

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
	bool OpenFile(const std::string& path, bool silent = false);
	bool SaveFile();
	bool SaveFileAs();
	void ResetGui();
	void ResetViewport();
	bool ExportClipboardRendering();
	bool LoadSettings();
	bool SaveSettings();

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
	void AddLine(const LineShape& line);
	void AddCircle(const CircleShape& circle);
	void AddArc(const ArcShape& arc);

	void RenderShapes();
};
