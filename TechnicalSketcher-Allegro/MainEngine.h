#pragma once

#include "pch.h"
#include "AllegroEngine.h"
#include "Shape.h"
#include "Layer.h"
#include "LayerArray.h"

enum CursorMode {
	MODE_SELECT,
	MODE_LINE,
	MODE_LINE_STRIP
};

class MainEngine : public AllegroEngine {
public:

	// User interface
	glm::vec2 panOffset = { 0, 0 };
	float scrollFactor = -0.2f;
	float scale = 50;
	float ctrl_snapTo = 1;
	float ctrl_currentLineThickness = 0.1;
	float ctrl_highlightCursorThreshold = 10;

	// Visuals
	float gridSub1Color = 220;	// Grayscale
	float gridSub2Color = 180;	// Grayscale
	float gridSub1Width = 1;
	float gridSub2Width = 1;
	glm::vec3 gfx_disabledLineColor = color(200);
	glm::vec3 gfx_normalLineColor = color(0);
	glm::vec3 gfx_hoveredLineColor = color(252, 132, 3);
	glm::vec3 gfx_selectedLineColor = color(255, 0, 0);
	glm::vec3 gfx_selectionBoxColor = color(252, 132, 3);
	glm::vec4 gfx_selectionBoxFillColor = color(200, 20, 0, 30);

	// Controls
	int cursorMode = MODE_SELECT;
	glm::vec2 mousePos = { 0, 0 };
	glm::vec2 mouseSnapped = { 0, 0 };

	glm::vec2 previewPoint = { 0, 0 };
	bool showPreviewPoint = false;
	glm::vec2 previewLineStart = { 0, 0 };
	bool draggingLine = false;
	bool draggingSelectionBox = false;		// Selection box also uses previewPoint position

	// Shapes
	LayerArray layers;
	size_t maxLayers = 0;
	ShapeID hoveredShape = -1;
	std::vector<ShapeID> selectedShapes;

	// Events
	size_t events_moveLayerBackID = -1;
	size_t events_moveLayerFrontID = -1;
	bool events_layerWindowWasHovered = false;

	bool events_layerSelectedEventFlag = false;
	bool events_layerWindowMouseEnteredEventFlag = false;

	// GUI
	const glm::ivec2 gui_leftUpperViewportCorner = { 200, 100 };
	const glm::ivec2 gui_hoverWindowSize = { 100, 100 };
	const int gui_layersBoxHeight = 200;
	const int gui_toolboxHeight = 200;
	bool gui_mouseOnGui = false;
	bool gui_previewChanged = true;		// Flag to indicate that previews need to be regenerated

	// Fonts

	ImFont* font_pt9 = nullptr;
	ImFont* font_pt10 = nullptr;
	ImFont* font_pt11 = nullptr;
	ImFont* font_pt12 = nullptr;
	ImFont* font_pt13 = nullptr;
	ImFont* font_pt14 = nullptr;
	ImFont* font_pt15 = nullptr;
	ImFont* font_pt16 = nullptr;
	ImFont* font_pt18 = nullptr;
	ImFont* font_pt20 = nullptr;
	ImFont* font_pt22 = nullptr;
	ImFont* font_pt25 = nullptr;
	ImFont* font_pt28 = nullptr;
	ImFont* font_pt30 = nullptr;
	ImFont* font_pt35 = nullptr;
	ImFont* font_pt40 = nullptr;






	// Main engine functionality, defined in MainEngine.cpp

	MainEngine();

	void setup() override;
	void draw() override;
	void destroy() override;
	void keyPressed(int keycode, unsigned int modifiers) override;
	void mouseScrolled(int x, int y) override;




	// ImGui fonts, defined in MainEngine_fonts.cpp

	void loadFonts();



	// Main logic functions, defined in MainEngine_logic.cpp

	void generateLayerPreviews();
	void addLayer();
	void addLayer(const std::string& name);
	void addLine(glm::vec2 p1, glm::vec2 p2);
	void changeMode(int mode);
	bool isShapeSelected(ShapeID shape);
	bool deleteShape(ShapeID shape);
	void cancelShape();





	// Events and callbacks, defined in MainEngine_Events.cpp

	void handleEvents();

	void OnMouseButtonLeftClicked();
	void OnMouseButtonRightClicked();
	void OnMouseButtonLeftReleased();
	void OnMouseButtonRightReleased();
	void OnMouseDragged();
	void OnMouseHovered();

	void OnLayerSelect(const Layer& layer);
	void OnMouseEnteredLayerWindow();





	// Graphics functions, defined in MainEngine_graphic.cpp

	void drawLayersOnScreen();
	void renderLayerToBitmap(LayerID layer, ALLEGRO_BITMAP* bitmap);

	// Returns an ALLEGRO_BITMAP* pointer, must be deleted with al_destroy_bitmap();
	ALLEGRO_BITMAP* createLayerPreviewBitmap(LayerID layer, int sizeX, int sizeY);

	void drawPreviewPoint(glm::vec2 pos);
	void drawLine(glm::vec2 from, glm::vec2 to, float thickness, glm::vec3 color);
	void drawOutlinedRectangle(glm::vec2 bottomleft, glm::vec2 topright, float outlineThickness, glm::vec3 color);
	void drawGrid();




	// GUI drawing, defined in MainEngine_GUI.cpp

	void drawLayersWindow();
	void drawToolboxWindow();
	void drawMousePositionWindow();

};