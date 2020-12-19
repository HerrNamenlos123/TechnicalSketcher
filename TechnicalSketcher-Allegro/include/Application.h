#pragma once

#include "pch.h"
#include "AllegroEngine.h"
#include "SketchFile.h"
#include "GuiWindows.h"
#include "json.hpp"

class Application : public AllegroEngine {
public:

	// User interface
	glm::vec2 panOffset = { 0, 0 };
	float scrollFactor = -0.2f;
	float scale = 50;
	float snapSize = 1;
	float ctrl_currentLineThickness = 0.1;
	float highlightDistanceToMouse = 8;

	// Visuals
	float gridLineColor = 220;	// Grayscale
	float gridLineWidth = 1;
	glm::vec3 gfx_disabledLineColor = color(200);
	glm::vec3 gfx_normalLineColor = color(0);
	glm::vec3 gfx_hoveredLineColor = color(252, 132, 3);
	glm::vec3 gfx_selectedLineColor = color(255, 0, 0);
	glm::vec3 gfx_selectionBoxColor = color(252, 132, 3);
	glm::vec4 gfx_selectionBoxFillColor = color(200, 20, 0, 30);

	// Controls
	enum CursorTool selectedTool = TOOL_SELECT;
	glm::vec2 mouse_workspace = { 0, 0 };
	glm::vec2 mouseSnapped_workspace = { 0, 0 };

	glm::vec2 previewPoint = { 0, 0 };
	bool showPreviewPoint = false;
	glm::vec2 previewLineStart = { 0, 0 };
	bool drawingLine = false;

	bool selectionBoxActive = false;
	glm::vec2 selectionBoxPointA = { 0, 0 };
	glm::vec2 selectionBoxPointB = { 0, 0 };

	std::vector<ShapeID> selectedShapes;
	ShapeID hoveredShape = -1;
	ShapeID lastHoveredShape = -1;

	// GUI
	GuiRibbonWindow ribbonWindow;
	GuiLayerWindow layerWindow;
	GuiToolboxWindow toolboxWindow;
	GuiMouseInfoWindow mouseInfoWindow;
	bool isMouseOnGui = false;

	// File
	SketchFile file;




	// Main engine events from the AllegroEngine, defined in Application.cpp

	Application();
	
	void setup() override;
	void draw() override;
	void destroy() override;
	void keyPressed(int keycode, int unicode, unsigned int modifiers, bool repeat) override;
	void mouseScrolled(int x, int y) override;




	// Main logic functions, defined in Application_logic.cpp

	void updateMousePositions();
	glm::vec2 convertScreenToWorkspaceCoords(glm::vec2 v);
	glm::vec2 convertWorkspaceToScreenCoords(glm::vec2 v);

	void generateLayerPreviews();
	void addLine(glm::vec2 p1, glm::vec2 p2);
	void changeMode(int mode);
	bool isShapeSelected(ShapeID shape);
	bool deleteShape(ShapeID shape);
	void cancelShape();
	void prepareGUI();

	std::vector<ShapeID> getHoveredShapes();
	ShapeID getClosestHoveredShape();

	// Unspecific mouse actions
	void mouseMoved();
	void mouseHovered();
	void mouseDragged();

	// Select tool
	void selectToolShapeClicked(ShapeID shape, bool ctrlKey);
	//void selectToolShapePointClicked(ShapeID shape, int pointNumber);
	void selectToolSpaceClicked(bool ctrlKey);
	void selectToolRightClicked();
	void selectToolShapeReleased(ShapeID shape, bool ctrlKey);
	//void selectToolShapePointReleased(ShapeID shape, int pointNumber);
	void selectToolSpaceReleased(bool ctrlKey);
	void selectToolMouseRightReleased();

	// Line tool
	void lineToolSpaceClicked();
	void lineToolRightClicked();
	void lineToolMouseReleased();
	void lineToolMouseRightReleased();

	// Line strip tool
	void lineStripToolSpaceClicked();
	void lineStripToolRightClicked();
	void lineStripToolMouseReleased();
	void lineStripToolMouseRightReleased();





	// Events and callbacks, defined in Application_Events.cpp

	void handleEvents();

	void OnMouseButtonLeftClicked();
	void OnMouseButtonRightClicked();
	void OnMouseButtonLeftReleased();
	void OnMouseButtonRightReleased();
	void OnMouseDragged();
	void OnMouseHovered();
	void OnMouseMoved();
	void OnMouseScrolled(float amount);

	void OnLayerSelect();
	void OnMouseEnteredLayerWindow();




	// Graphics functions, defined in Application_graphic.cpp

	void renderApplication();
	void renderShapes();
	void renderLayerToBitmap(Layer* layer, ALLEGRO_BITMAP* bitmap);
	void drawLittlePoint(glm::vec2 pos, float size);
	void drawLine(glm::vec2 from, glm::vec2 to, float thickness, glm::vec3 color);
	void drawOutlinedRectangle(glm::vec2 bottomleft, glm::vec2 topright, float outlineThickness, glm::vec3 color);
	void drawGrid();




	// File loading and saving, defined in Application_files.cpp

	void updateWindowTitle();
	bool closeFile();
	bool openFile();
	bool saveFile(bool saveAs = false);

};