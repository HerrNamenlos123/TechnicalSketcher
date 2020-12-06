
#include "pch.h"
#include "Engine.h"
#include "Shape.h"
#include "misc.h"

#include "imgui/imgui_internal.h"

template<typename T>
void print(T msg) {
	std::cout << msg << std::endl;
}

template<typename T, typename... Trecurse>
void print(T msg, Trecurse... recurse) {
	std::cout << msg;
	print(recurse...);
}

enum CursorMode {
	MODE_SELECT,
	MODE_LINE,
	MODE_LINE_STRIP
};

class Layer {

	std::vector<Shape> shapes;
	ShapeID maxShapeID = 0;

public:
	std::string name;
	ALLEGRO_BITMAP* bitmap = nullptr;

	Layer(const std::string& n) {
		name = n;
		bitmap = nullptr;
	}

	Layer(const Layer& layer) {
		name = layer.name;
		shapes = layer.shapes;
		bitmap = nullptr;
	};

	void operator=(const Layer& layer) {
		name = layer.name;
		shapes = layer.shapes;
		bitmap = nullptr;
	};

	~Layer() {
		al_destroy_bitmap(bitmap);
		bitmap = nullptr;
	}

	void cloneFrom(ALLEGRO_BITMAP* bmp) {
		al_destroy_bitmap(bitmap);
		bitmap = al_clone_bitmap(bmp);
	}

	void addShape(enum ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness) {

		ShapeID id = maxShapeID;
		maxShapeID++;

		shapes.push_back(Shape(id, type, p1, p2, thickness));
	}
	
	bool removeShape(ShapeID shape) {

		for (int i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == shape) {
				shapes.erase(shapes.begin() + i);
				return true;
			}
		}

		return false;
	}

	std::vector<Shape>& getShapes() {
		return shapes;
	}

	size_t size() {
		return shapes.size();
	}

	Shape getShape(ShapeID shape) {
		
		for (size_t i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == shape) {
				return shapes[i];
			}
		}

		return Shape();
	}
};
/*
template<typename T>
class SwitchableSelectables {

	std::vector<T> fields;
	std::vector<size_t> id;

public:
	SwitchableSelectables() {
	}

	void addFieldFront(const T& field, const std::string& name) {
		fields.insert(fields.begin(), field);
		names.insert(names.begin(), field);
	}

	void addFieldBack(const T& field, const std::string& name) {
		fields.push_back(field);
		names.push_back(name);
	}

	bool insertField(const T& field, const std::string& name, size_t index) {
		if (index < fields.size()) {
			fields.insert(fields.begin() + index, field);
			names.insert(names.begin() + index, name);
			return true;
		}
		return false;
	}

	bool moveFieldFront(size_t index) {
		if (index > 0 && index < fields.size()) {
			std::iter_swap(fields.begin() + index, fields.begin() + index - 1);
			std::iter_swap(fields.begin() + index, fields.begin() + index - 1);
		}
	}

	bool moveFieldBack(size_t index) {
		if (index > 0 && index < fields.size()) {
			std::iter_swap(fields.begin() + index, fields.begin() + index + 1);
		}
	}

	void draw() {

	}
};*/

class Engine : public AllegroEngine {

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
	std::vector<Layer> layers;
	size_t maxLayers = 0;
	int selectedLayer = 0;
	ShapeID hoveredShape = -1;
	std::vector<ShapeID> selectedShapes;


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

	// GUI
	glm::ivec2 gui_leftUpperViewportCorner = { 200, 100 };
	glm::ivec2 gui_hoverWindowSize = { 100, 100 };
	int gui_layersBoxHeight = 200;
	int gui_toolboxHeight = 200;
	bool gui_mouseOnGui = false;
	bool gui_previewChanged = true;		// Flag to indicate that previews need to be regenerated

public:
	Engine() {
	}

	void setup() override {
		changeMode(MODE_SELECT);
		addLayer();

		ImGuiIO& io = ImGui::GetIO();
		font_pt9  = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 9);
		font_pt10 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 10);
		font_pt11 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 11);
		font_pt12 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 12);
		font_pt13 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 13);
		font_pt14 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 14);
		font_pt15 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 15);
		font_pt16 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 16);
		font_pt18 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 18);
		font_pt20 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 20);
		font_pt22 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 22);
		font_pt25 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 25);
		font_pt28 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 28);
		font_pt30 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 30);
		font_pt35 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 35);
		font_pt40 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 40);

		generateLayerPreviews();
		showPreviewPoint = false;
	}

	void draw() override {

		// Updating all logic

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		//window_flags |= ImGuiWindowFlags_NoBackground;
		//window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::PushFont(font_pt20);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(width, gui_leftUpperViewportCorner.y));
		bool isOpen = true;
		ImGui::GetStyle().WindowRounding = 0.0f;

		ImGui::Begin("TopBar", &isOpen, window_flags);
		ImGui::End();

		ImGui::ShowDemoWindow();
		drawLayersWindow();
		drawToolboxWindow();
		drawMousePositionWindow();

		ImGui::PopFont();
		ImGuiIO& io = ImGui::GetIO();
		gui_mouseOnGui = io.WantCaptureMouse;

		updateControls();



		// Now everything is calculated, display it now

		background(255);

		if (draggingSelectionBox) {	// Fill should be in the back
			filledRectangle(previewPoint, mouse, gfx_selectionBoxFillColor);
		}

		drawGrid();
		drawShapes();

		if (draggingLine) {
			drawLine(previewLineStart, mouseSnapped, ctrl_currentLineThickness, color(0));
		}
		if (showPreviewPoint) {
			drawPreviewPoint(previewPoint);
		}
		if (draggingSelectionBox) {	// Outline in the front
			outlinedRectangle(previewPoint, mouse, gfx_selectionBoxColor, 1);
		}
	}

	void destroy() override {
	}

	void keyPressed(int keycode, unsigned int modifiers) override {
		if (keycode == ALLEGRO_KEY_ESCAPE) {
			close();
		}

		if (keycode == ALLEGRO_KEY_DELETE) {

			// Delete all selected shapes
			for (size_t i = 0; i < selectedShapes.size(); i++) {
				deleteShape(selectedShapes[i]);
			}
			selectedShapes.clear();
		}
		else if (keycode == ALLEGRO_KEY_UP) {

			std::vector<Shape>& shapes = layers[selectedLayer].getShapes();

			for (size_t i = 0; i < selectedShapes.size(); i++) {
				for (Shape& shape : shapes) {
					if (shape.shapeID == selectedShapes[i]) {
						shape.p1.y -= ctrl_snapTo;
						shape.p2.y -= ctrl_snapTo;
						break;
					}
				}
			}

		}
		else if (keycode == ALLEGRO_KEY_DOWN) {

			std::vector<Shape>& shapes = layers[selectedLayer].getShapes();

			for (size_t i = 0; i < selectedShapes.size(); i++) {
				for (Shape& shape : shapes) {
					if (shape.shapeID == selectedShapes[i]) {
						shape.p1.y += ctrl_snapTo;
						shape.p2.y += ctrl_snapTo;
						break;
					}
				}
			}

		}
		else if (keycode == ALLEGRO_KEY_RIGHT) {

			std::vector<Shape>& shapes = layers[selectedLayer].getShapes();

			for (size_t i = 0; i < selectedShapes.size(); i++) {
				for (Shape& shape : shapes) {
					if (shape.shapeID == selectedShapes[i]) {
						shape.p1.x += ctrl_snapTo;
						shape.p2.x += ctrl_snapTo;
						break;
					}
				}
			}

		}
		else if (keycode == ALLEGRO_KEY_LEFT) {

			std::vector<Shape>& shapes = layers[selectedLayer].getShapes();

			for (size_t i = 0; i < selectedShapes.size(); i++) {
				for (Shape& shape : shapes) {
					if (shape.shapeID == selectedShapes[i]) {
						shape.p1.x -= ctrl_snapTo;
						shape.p2.x -= ctrl_snapTo;
						break;
					}
				}
			}

		}
	}

	void mouseScrolled(int x, int y) override {

		if (gui_mouseOnGui)
			return;

		float scroll = (x + y) * scrollFactor;
		float factor = 1 + std::abs(scroll);

		if (scroll > 0) {
			scale *= factor;
		}
		else {
			scale /= factor;
		}

		glm::vec2 mouseToCenter = glm::vec2(panOffset.x - mouse.x + width / 2.f, panOffset.y - mouse.y + height / 2.f);

		if (scroll > 0)
			panOffset += mouseToCenter * factor - mouseToCenter;
		else
			panOffset -= mouseToCenter - mouseToCenter / factor;
	}



	// Gui functions

	void drawLayersWindow() {

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowPos(ImVec2(0, gui_leftUpperViewportCorner.y));
		ImGui::SetNextWindowSize(ImVec2(gui_leftUpperViewportCorner.x, gui_layersBoxHeight));
		bool isOpen = true;
		ImGui::GetStyle().WindowRounding = 0.0f;

		ImGui::Begin("LeftBarLayers", &isOpen, window_flags);
		{
			ImGui::PushFont(font_pt16);
			ImGui::Text("Layers"); ImGui::SameLine();
			ImGui::SetCursorPosX(gui_leftUpperViewportCorner.x * 0.8);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			if (ImGui::Button("+##AddLayer", ImVec2(25, 17))) {
				addLayer();
			}
			ImGui::PopStyleVar();
			ImGui::Separator();
			ImGui::Columns(2);

			int _moveLayerUp = -1;
			int _moveLayerDown = -1;
			for (size_t i = 0; i < layers.size(); i++) {
				if (ImGui::Selectable(layers[i].name.c_str(), selectedLayer == static_cast<int>(i))) {
					selectLayer(static_cast<int>(i));
				}

				// Draw the preview of the layer
				if (ImGui::IsItemHovered())
				{
					if (gui_previewChanged) {
						generateLayerPreviews();	// Flag is reset in there
					}

					if (layers[i].bitmap != nullptr) {
						ImGui::BeginTooltip();
						ImGui::Image(layers[i].bitmap, ImVec2(gui_hoverWindowSize.x, gui_hoverWindowSize.y));
						ImGui::EndTooltip();
					}
				}


				ImGui::PushFont(font_pt16);
				ImGui::NextColumn();
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				if (i == 0) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}
				bool up = ImGui::Button(("^##" + layers[i].name).c_str(), ImVec2(25, 17)); ImGui::SameLine();
				if (i == 0) {
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
				if (i == layers.size() - 1) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}
				bool down = ImGui::Button(("v##" + layers[i].name).c_str(), ImVec2(25, 17));
				if (i == layers.size() - 1) {
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
				
				ImGui::PopStyleVar();
				ImGui::NextColumn();
				ImGui::PopFont();

				if (up) {
					_moveLayerUp = static_cast<int>(i);
				}
				if (down) {
					_moveLayerDown = static_cast<int>(i);
				}
			}
			ImGui::PopFont();

			if (_moveLayerUp != -1) {
				moveLayerUp(_moveLayerUp);
			}
			if (_moveLayerDown != -1) {
				moveLayerDown(_moveLayerDown);
			}
		}
		ImGui::End();
	}

	void drawToolboxWindow() {

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowPos(ImVec2(0, gui_leftUpperViewportCorner.y + gui_toolboxHeight));
		ImGui::SetNextWindowSize(ImVec2(gui_leftUpperViewportCorner.x, gui_toolboxHeight));
		bool isOpen = true;
		ImGui::GetStyle().WindowRounding = 0.0f;

		ImGui::Begin("ToolBox", &isOpen, window_flags);
		{
			ImGui::PushFont(font_pt16);
			if (ImGui::Selectable("Selection mode", cursorMode == MODE_SELECT)) {
				changeMode(MODE_SELECT);
			}
			if (ImGui::Selectable("Line mode", cursorMode == MODE_LINE)) {
				changeMode(MODE_LINE);
			}
			if (ImGui::Selectable("Line strip mode", cursorMode == MODE_LINE_STRIP)) {
				changeMode(MODE_LINE_STRIP);
			}
			ImGui::PopFont();
		}
		ImGui::End();
	}

	void drawMousePositionWindow() {

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		//window_flags |= ImGuiWindowFlags_NoBackground;

		int h = 30;
		ImGui::SetNextWindowPos(ImVec2(0, height - h));
		ImGui::SetNextWindowSize(ImVec2(gui_leftUpperViewportCorner.x, h));
		bool isOpen = true;
		ImGui::GetStyle().WindowRounding = 0.0f;

		ImGui::Begin("MouseInfo", &isOpen, window_flags);
		{
			ImGui::PushFont(font_pt11);

			std::stringstream text;
			text << std::fixed << std::setprecision(2) << "Mouse: ";
			if (mousePos.x >= 0)
				text << " ";
			text << mousePos.x << "|";
			if (mousePos.y >= 0)
				text << " ";
			text << mousePos.y << " Snap: " << std::setprecision(0);
			if (mouseSnapped.x >= 0)
				text << " ";
			text << mouseSnapped.x << "|";
			if (mouseSnapped.y >= 0)
				text << " ";
			text << mouseSnapped.y;
			ImGui::Text(text.str().c_str());

			ImGui::PopFont();
		}
		ImGui::End();
	}




	// Usage functions

	void generateLayerPreviews() {
		timestamp_t start = micros();

		for (size_t i = 0; i < layers.size(); i++) {
			ALLEGRO_BITMAP* bitmap = createLayerPreviewBitmap(static_cast<int>(i), gui_hoverWindowSize.x, gui_hoverWindowSize.y);
			layers[i].cloneFrom(bitmap);
			al_destroy_bitmap(bitmap);
		}
		gui_previewChanged = false;
		std::cout << "Generated preview bitmaps, took " << (micros() - start) / 1000.0 << " ms" << std::endl;
	}

	void moveLayerUp(int layer) {
		if (layer > 0) {

			cancelShape();

			// Swap two adjacent layers
			std::iter_swap(layers.begin() + layer, layers.begin() + layer - 1);

			// Make sure the selected layer stays the same after swapping
			if (selectedLayer == layer)
				selectedLayer--;
			else if (selectedLayer == layer - 1)
				selectedLayer++;

			gui_previewChanged = true;
		}
	}

	void moveLayerDown(int layer) {
		if (layer < layers.size() - 1) {

			cancelShape();

			// Swap two adjacent layers
			std::iter_swap(layers.begin() + layer, layers.begin() + layer + 1);

			// Make sure the selected layer stays the same after swapping
			if (selectedLayer == layer)
				selectedLayer++;
			else if (selectedLayer == layer + 1)
				selectedLayer--;

			gui_previewChanged = true;
		}
	}

	void addLayer() {
		addLayer("Layer #" + std::to_string(maxLayers));
	}

	void addLayer(const std::string& name) {

		cancelShape();

		layers.insert(layers.begin(), Layer(name));
		maxLayers++;

		gui_previewChanged = true;
	}

	void selectLayer(int layer) {
		if (layer < layers.size()) {
			cancelShape();
			selectedLayer = layer;
			selectedShapes.clear();
		}
		else {
			throw std::logic_error("Chosen layer does not exist!");
		}
	}

	void addLine(glm::vec2 p1, glm::vec2 p2) {
		if (p1 != p2) {
			if (selectedLayer < layers.size()) {
				layers[selectedLayer].addShape(SHAPE_LINE, p1, p2, ctrl_currentLineThickness);
			}
			else {
				throw std::logic_error("Chosen layer does not exist!");
			}
		}

		gui_previewChanged = true;
	}

	void changeMode(int mode) {

		cancelShape();
		selectedShapes.clear();

		switch (mode) {

		case MODE_SELECT:
			cursorMode = MODE_SELECT;
			showPreviewPoint = false;
			draggingSelectionBox = false;
			break;

		case MODE_LINE:
			cursorMode = MODE_LINE;
			previewPoint = mouseSnapped;
			showPreviewPoint = true;
			draggingLine = false;
			draggingSelectionBox = false;
			break;

		case MODE_LINE_STRIP:
			cursorMode = MODE_LINE_STRIP;
			previewPoint = mouseSnapped;
			showPreviewPoint = true;
			draggingLine = false;
			draggingSelectionBox = false;
			break;

		default:
			break;
		}
	}

	bool isShapeSelected(ShapeID shape) {

		for (size_t i = 0; i < selectedShapes.size(); i++) {
			if (selectedShapes[i] == shape) {
				return true;
			}
		}

		return false;
	}

	bool deleteShape(ShapeID shape) {
		gui_previewChanged = true;
		
		return layers[selectedLayer].removeShape(shape);
	}



	// Control events

	void mouseLeftPressed() {

		if (gui_mouseOnGui)
			return;

		if (cursorMode == MODE_LINE || cursorMode == MODE_LINE_STRIP) {

			if (!draggingLine) {
				draggingLine = true;
				showPreviewPoint = false;
				previewLineStart = mouseSnapped;
			}
			else {

				if (cursorMode == MODE_LINE_STRIP) {
					if (mouseSnapped != previewLineStart) {
						addLine(mouseSnapped, previewLineStart);
					}
					showPreviewPoint = false;
					draggingLine = true;
					previewLineStart = mouseSnapped;
				}
				else {
					showPreviewPoint = true;
					draggingLine = false;
					previewPoint = mouseSnapped;
					if (mouseSnapped != previewLineStart) {
						addLine(mouseSnapped, previewLineStart);
					}
				}

			}
		}
		else if (cursorMode == MODE_SELECT) {

			bool ctrl = getKey(ALLEGRO_KEY_LCTRL);

			if (hoveredShape == -1) {
				if (!ctrl) {	// Prevent unselecting everything when missing the line
					selectedShapes.clear();

					// Start drawing selection box
					draggingSelectionBox = true;
					previewPoint = mouse;
					showPreviewPoint = false;
				}
			}
			else {

				if (ctrl) {	// CTRL Key is held

					if (isShapeSelected(hoveredShape)) {	// If already selected, unselect

						// Remove from the list
						for (size_t i = 0; i < selectedShapes.size(); i++) {
							if (selectedShapes[i] == hoveredShape) {
								selectedShapes.erase(selectedShapes.begin() + i);
							}
						}
					}
					else {
						selectedShapes.push_back(hoveredShape);
					}
				}
				else {
					selectedShapes.clear();
					selectedShapes.push_back(hoveredShape);
				}
			}
		}
	}

	void mouseRightPressed() {
		cancelShape();
	}

	void mouseLeftReleased() {

		// Not caring when mouse is on UI

		// Stop drawing selection box
		if (draggingSelectionBox) {
			draggingSelectionBox = false;
			selectedShapes.clear();

			glm::vec2 prev = (previewPoint - glm::vec2(width, height) * 0.5f - panOffset) / scale;

			float left = std::min(prev.x, mousePos.x);
			float right = std::max(prev.x, mousePos.x);
			float top = std::max(prev.y, mousePos.y);
			float bottom = std::min(prev.y, mousePos.y);

			for (Shape& shape : layers[selectedLayer].getShapes()) {
				if (shape.type == SHAPE_LINE) {
					if (shape.p1.x >= left && shape.p1.x <= right && shape.p1.y >= bottom && shape.p1.y <= top &&
						shape.p2.x >= left && shape.p2.x <= right && shape.p2.y >= bottom && shape.p2.y <= top)
					{
						// Line is fully contained in the selection box
						selectedShapes.push_back(shape.shapeID);
					}
				}
			}
		}
	}

	void mouseRightReleased() {

		if (gui_mouseOnGui)
			return;
	}

	void mouseDragged() {

		if (gui_mouseOnGui)
			return;

		if (cursorMode == MODE_SELECT && mouseWheelPressed) {
			panOffset.x += mouse.x - pmouse.x;
			panOffset.y += mouse.y - pmouse.y;
		}
	}

	void mouseHovered() {

		if (gui_mouseOnGui)
			return;

		draggingSelectionBox = false;

		if (cursorMode == MODE_LINE || cursorMode == MODE_LINE_STRIP) {
			if (!draggingLine) {
				previewPoint = mouseSnapped;
			}
		}
		else if (cursorMode == MODE_SELECT) {

			hoveredShape = -1;
			Layer& layer = layers[selectedLayer];

			if (layer.size() > 0) {
				for (int i = static_cast<int>(layer.size()) - 1; i >= 0; i--) {
					Shape& shape = layer.getShapes()[i];

					glm::vec2 p1 = panOffset + shape.p1 * scale + glm::vec2(width, height) * 0.5f;
					glm::vec2 p2 = panOffset + shape.p2 * scale + glm::vec2(width, height) * 0.5f;

					if (shape.type == SHAPE_LINE) {
						if (getDistanceAroundLine(p1, p2, mouse) <= ctrl_highlightCursorThreshold) {
							hoveredShape = shape.shapeID;
							break;
						}
					}
				}
			}
		}
	}

	void cancelShape() {
		showPreviewPoint = true;
		draggingLine = false;
	}






	// Main control distributor

	void updateControls() {

		mouseButtonRightPressed && !pmouseButtonRightPressed;

		mousePos = glm::vec2((mouse.x - panOffset.x - width / 2.f) / scale, (mouse.y - panOffset.y - height / 2.f) / scale);
		mouseSnapped = glm::vec2(round(mousePos.x / ctrl_snapTo) * ctrl_snapTo, round(mousePos.y / ctrl_snapTo) * ctrl_snapTo);

		if (mouseButtonLeftPressed && !pmouseButtonLeftPressed) {
			mouseLeftPressed();
		}

		if (mouseButtonRightPressed && !pmouseButtonRightPressed) {
			mouseRightPressed();
		}

		if (!mouseButtonLeftPressed && pmouseButtonLeftPressed) {
			mouseLeftReleased();
		}

		if (!mouseButtonRightPressed && pmouseButtonRightPressed) {
			mouseRightReleased();
		}

		if (mousePressed && pmouse != mouse) {
			mouseDragged();
		}

		if (!mousePressed) {
			mouseHovered();
		}
	}





	// Drawing functions

	void drawShapes() {

		for (int layer = static_cast<int>(layers.size() - 1); layer >= 0; layer--) {
			for (size_t i = 0; i < layers[layer].size(); i++) {

				Shape& shape = layers[layer].getShapes()[i];

				switch (shape.type) {
				case SHAPE_LINE:

					if (layer == selectedLayer) {

						// Shape is selected
						if (isShapeSelected(shape.shapeID)) {
							if (shape.shapeID == hoveredShape) {	// Shape is selected and hovered
								drawLine(shape.p1, shape.p2, shape.thickness, (gfx_hoveredLineColor + gfx_selectedLineColor) / 2.f);
							}
							else {
								drawLine(shape.p1, shape.p2, shape.thickness, gfx_selectedLineColor);
							}
						}
						else { // Shape is simply hovered
							if (shape.shapeID == hoveredShape) {
								drawLine(shape.p1, shape.p2, shape.thickness, gfx_hoveredLineColor);
							}
							else {
								drawLine(shape.p1, shape.p2, shape.thickness, gfx_normalLineColor);
							}
						}
					}
					else {
						drawLine(shape.p1, shape.p2, shape.thickness, gfx_disabledLineColor);
					}
					break;

				case SHAPE_CIRCLE:
					break;

				default:
					break;
				}
			}
		}
	}

	void renderLayerToBitmap(int _layer, ALLEGRO_BITMAP* bitmap) {
		Layer& layer = layers[_layer];
		std::vector<Shape>& shapes = layer.getShapes();

		int sizeX = al_get_bitmap_width(bitmap);
		int sizeY = al_get_bitmap_height(bitmap);

		// If no shapes, just render white
		if (layer.size() == 0) {
			al_set_target_bitmap(bitmap);
			al_clear_to_color(al_color(color(255)));
			al_set_target_bitmap(al_get_backbuffer(display));
			return;
		}

		// Calculate the encapsulated frame
		float leftMost = shapes[0].p1.x;
		float rightMost = shapes[0].p1.x;
		float bottomMost = shapes[0].p1.y;
		float topMost = shapes[0].p1.y;

		for (Shape& s : shapes) {

			if (s.p1.x < leftMost)
				leftMost = s.p1.x;
			if (s.p1.x > rightMost)
				rightMost = s.p1.x;
			if (s.p1.y < bottomMost)
				bottomMost = s.p1.y;
			if (s.p1.y > topMost)
				topMost = s.p1.y;

			if (s.p2.x < leftMost)
				leftMost = s.p2.x;
			if (s.p2.x > rightMost)
				rightMost = s.p2.x;
			if (s.p2.y < bottomMost)
				bottomMost = s.p2.y;
			if (s.p2.y > topMost)
				topMost = s.p2.y;
		}

		glm::vec2 sourceFrameSize = { rightMost - leftMost, topMost - bottomMost };
		glm::vec2 renderFrameSize = { sizeX, sizeY };

		// Find separate scaling factors
		float scaleX = NAN;
		if (sourceFrameSize.x != 0)
			scaleX = sizeX / sourceFrameSize.x;
		float scaleY = NAN;
		if (sourceFrameSize.y != 0)
			scaleY = sizeY / sourceFrameSize.y;

		float mappedLeft = 0;
		float mappedRight = 0;
		float mappedTop = 0;
		float mappedBottom = 0;
		// X size is larger
		if (scaleX <= scaleY || (!isnan(scaleX) && isnan(scaleY))) {
			mappedLeft = leftMost;
			mappedRight = rightMost;
			mappedTop = (topMost + bottomMost) / 2.f + (leftMost - rightMost) * (static_cast<float>(sizeY) / sizeX) / 2.f;
			mappedBottom = (topMost + bottomMost) / 2.f - (leftMost - rightMost) * (static_cast<float>(sizeY) / sizeX) / 2.f;
		}
		else if (scaleX > scaleY || (isnan(scaleX) && !isnan(scaleY))) {	// Y size is larger
			mappedTop = bottomMost;
			mappedBottom = topMost;
			mappedLeft = (rightMost + leftMost) / 2.f + (bottomMost - topMost) / (static_cast<float>(sizeY) / sizeX) / 2.f;
			mappedRight = (rightMost + leftMost) / 2.f - (bottomMost - topMost) / (static_cast<float>(sizeY) / sizeX) / 2.f;
		}
		else {	// Both are NAN
			al_set_target_bitmap(bitmap);
			al_clear_to_color(al_color(color(255)));
			al_set_target_bitmap(al_get_backbuffer(display));
			return;
		}

		float mappedWidth = mappedRight - mappedLeft;
		float mappedHeight = mappedTop - mappedBottom;
		float mappedCenterX = (mappedRight + mappedLeft) / 2.f;
		float mappedCenterY = (mappedTop + mappedBottom) / 2.f;

		float brim = 1.3f;
		mappedLeft = mappedCenterX - mappedWidth / 2.f * brim;
		mappedRight = mappedCenterX + mappedWidth / 2.f * brim;
		mappedBottom = mappedCenterY - mappedHeight / 2.f * brim;
		mappedTop = mappedCenterY + mappedHeight / 2.f * brim;

		//framebl1 = { mappedLeft, mappedBottom };
		//frametr1 = { mappedRight, mappedTop };
		//framebl2 = { leftMost, bottomMost };
		//frametr2 = { rightMost, topMost };
		//framebl3 = { mappedCenterX, mappedCenterY };
		//frametr3 = { mappedRight, mappedTop };

		// Save current draw buffer to return to later
		ALLEGRO_BITMAP* previousBuffer = al_get_target_bitmap();
		al_set_target_bitmap(bitmap);
		al_clear_to_color(al_color(color(255)));

		for (Shape& s : shapes) {

			glm::vec2 scaled1;
			scaled1.x = mapFloat(s.p1.x, mappedLeft, mappedRight, 0, sizeX);
			scaled1.y = mapFloat(s.p1.y, mappedTop, mappedBottom, 0, sizeY);
			glm::vec2 scaled2;
			scaled2.x = mapFloat(s.p2.x, mappedLeft, mappedRight, 0, sizeX);
			scaled2.y = mapFloat(s.p2.y, mappedTop, mappedBottom, 0, sizeY);

			float thickness = mapFloat(s.thickness, 0, mappedWidth, 0, sizeX);
			fancyLine(scaled1, scaled2, color(0), thickness);
		}

		al_set_target_bitmap(previousBuffer);
	}

	// 
	// Returns an ALLEGRO_BITMAP* pointer, must be deleted with al_destroy_bitmap();
	//
	ALLEGRO_BITMAP* createLayerPreviewBitmap(int layer, int sizeX, int sizeY) {

		ALLEGRO_BITMAP* bitmap = al_create_bitmap(sizeX, sizeY);
		if (bitmap == nullptr) {
			return nullptr;
		}

		renderLayerToBitmap(layer, bitmap);

		return bitmap;
	}

	void drawPreviewPoint(glm::vec2 pos) {
		glm::vec2 p = panOffset + pos * scale + glm::vec2(width, height) * 0.5f;

		outlinedFilledRectangleCentered(p, { 4, 4 }, color(255), color(0), 1);
	}

	void drawLine(glm::vec2 from, glm::vec2 to, float thickness, glm::vec3 color) {
		glm::vec2 p1 = panOffset + from * scale + glm::vec2(width, height) * 0.5f;
		glm::vec2 p2 = panOffset + to * scale + glm::vec2(width, height) * 0.5f;

		fancyLine(p1, p2, color, thickness * scale);
	}

	void drawOutlinedRectangle(glm::vec2 bottomleft, glm::vec2 topright, float outlineThickness, glm::vec3 color) {
		glm::vec2 p1 = panOffset + bottomleft * scale + glm::vec2(width, height) * 0.5f;
		glm::vec2 p2 = panOffset + topright * scale + glm::vec2(width, height) * 0.5f;

		fancyLine({ p1.x, p1.y }, { p2.x, p1.y }, color, outlineThickness * scale);
		fancyLine({ p2.x, p1.y }, { p2.x, p2.y }, color, outlineThickness * scale);
		fancyLine({ p2.x, p2.y }, { p1.x, p2.y }, color, outlineThickness * scale);
		fancyLine({ p1.x, p2.y }, { p1.x, p1.y }, color, outlineThickness * scale);
	}

	void drawGrid() {

		float thickness = gridSub1Width;
		float col = gridSub1Color;

		// Sub grid lines
		if (scale * ctrl_snapTo > 3) {
			for (float x = panOffset.x + width / 2; x < width; x += scale * ctrl_snapTo) {
				line({ x, 0 }, { x, height }, color(col), thickness);
			}
			for (float x = panOffset.x + width / 2; x > 0; x -= scale * ctrl_snapTo) {
				line({ x, 0 }, { x, height }, color(col), thickness);
			}
			for (float y = panOffset.y + height / 2; y < height; y += scale * ctrl_snapTo) {
				line({ 0, y }, { width, y }, color(col), thickness);
			}
			for (float y = panOffset.y + height / 2; y > 0; y -= scale * ctrl_snapTo) {
				line({ 0, y }, { width, y }, color(col), thickness);
			}

			thickness = gridSub2Width;
			col = gridSub2Color;
		}

		// Bigger grid lines
		//if (scale > 1) {
		//	for (float x = panOffset.x; x < width / 2; x += scale * 10) {
		//		primitives.line(x, -height / 2.f, x, height / 2.f);
		//	}
		//	for (float x = panOffset.x; x > -width / 2; x -= scale * 10) {
		//		primitives.line(x, -height / 2.f, x, height / 2.f);
		//	}
		//	for (float y = panOffset.y; y < height / 2; y += scale * 10) {
		//		primitives.line(-width / 2.f, y, width / 2.f, y);
		//	}
		//	for (float y = panOffset.y; y > -height / 2; y -= scale * 10) {
		//		primitives.line(-width / 2.f, y, width / 2.f, y);
		//	}
		//}

		// Coordinate center
		//primitives.strokeWeight(3);
		//primitives.stroke(color(gridSub2Color));
		//primitives.line(panOffset.x, panOffset.y, panOffset.x + 20, panOffset.y);
		//primitives.line(panOffset.x, panOffset.y, panOffset.x, panOffset.y + 20);
	}
};

int main() {

	Engine engine;
	engine.run(800, 600);

	return 0;
}