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

enum class UpdateStatus {
	NOTHING,			// This is the default
	//USER_PROMPT,
	INITIALIZING,
	DOWNLOADING,
	EXTRACTING,
	INSTALLING,
	DONE,				// Request to restart the application
	FAILED
};

class Navigator {

	Navigator() {}

	inline static std::unique_ptr<Navigator> instance;

public:
	Navigator(Navigator const&) = delete;
	void operator=(Navigator const&) = delete;

	SketchFile m_file;
	bool fileChanged = false;
	semver::version m_applicationVersion;
	b::fs::path m_imguiFileLocation;

	ImVec2 m_windowSize = ImVec2(0, 0);	// Retrieve every frame, to be consistent through the update loop
	//Battery::ClipboardFormatID clipboardShapeFormat = 0;	// TODO: windowSize is maybe unnecessary

	// User interface
	ImVec2 m_panOffset = { 0, 0 };
	float scrollFactor = 0.2f;
	float scale = 7;
	float defaultSnapSize = 5;
	float snapSize = defaultSnapSize;
	float mouseHighlightThresholdDistance = 8;
	bool infiniteSheet = false;
	bool gridShown = true;
	
	std::atomic<enum class UpdateStatus> updateStatus = UpdateStatus::NOTHING;
	std::atomic<sf::Time> timeSincePopup;		// Timestamp when the popup was created, set by Updater
	std::atomic<double> updateProgress = 0.0;		// 0.0 to 1.0
	b::fs::path restartExecutablePath;

	float exportDPI = 300;              // To be implemented
	bool exportTransparent = true;      // To be implemented
	bool popupExportOpen = false;       // To be implemented
	bool popupDeleteLayerOpen = false;
	bool popupSettingsOpen = false;
	bool keepUpToDate = true;           // To be implemented
	bool tabbedShapeInfo = false;

	// Buffers to store event data
	std::vector<sf::Event> eventBuffer;

	// Controls
	ImVec2 mousePosition = { 0, 0 };			// Those mouse positions are in workspace coordinates
	ImVec2 mouseSnapped = { 0, 0 };
	bool controlKeyPressed = false;
	bool shiftKeyPressed = false;

	ImVec2 previewPointPosition = { 0, 0 };
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
	void OnEvent(sf::Event e, bool& handled);


	ImVec2 ConvertScreenToWorkspaceCoords(const ImVec2& v);
	ImVec2 ConvertWorkspaceToScreenCoords(const ImVec2& v);
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

	void OnKeyPressed(sf::Event event);
	void OnKeyReleased(sf::Event event);
	void OnMouseClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel);
	void OnMouseReleased(const ImVec2& position, bool left, bool right, bool wheel);
	void OnMouseMoved(const ImVec2& position, const ImVec2& snapped, int dx, int dy);
	void OnMouseHovered(const ImVec2& position, const ImVec2& snapped, int dx, int dy);
	void OnMouseDragged(const ImVec2& position, const ImVec2& snapped, int dx, int dy);
	void OnSpaceClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel);
	void OnShapeClicked(const ImVec2& position, const ImVec2& snapped, bool left, bool right, bool wheel, ShapeID shape);
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
	bool OpenFile(const b::fs::path& path, bool silent = false);
	bool SaveFile();
	bool SaveFileAs();
	void ResetGui();
	void ResetViewport();
	bool ExportToClipboard();
	bool ExportToFile();

	semver::version GetApplicationVersion();
	void OpenNewWindowFile(const b::fs::path& filepath);
	void StartNewApplicationInstance();
	bool CloseApplication();

	void AddLayer();
	void AddLine(const LineShape& line);
	void AddCircle(const CircleShape& circle);
	void AddArc(const ArcShape& arc);

	void RenderShapes();
};
