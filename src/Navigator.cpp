
#include "Navigator.h"
#include "Layer.h"
#include "config.h"

Navigator::Navigator() {

}

void Navigator::OnAttach() {
	renderScene = new Battery::Scene(&applicationPointer->window);
}

void Navigator::OnDetach() {
	delete renderScene;
}

void Navigator::OnUpdate() {
	windowSize = glm::ivec2(applicationPointer->window.GetWidth(), applicationPointer->window.GetHeight());
}

void Navigator::OnRender() {
	using namespace Battery;
	Renderer2D::BeginScene(renderScene);
	Renderer2D::DrawBackground(BACKGROUND_COLOR);

	// Draw first part of the selection box
	if (selectionBoxActive) {			// Fill should be in the back
		glm::vec2 p1 = ConvertWorkspaceToScreenCoords(selectionBoxPointA);
		glm::vec2 p2 = ConvertWorkspaceToScreenCoords(selectionBoxPointB);
		//filledRectangle(p1, p2, gfx_selectionBoxFillColor);
		LOG_ERROR("DRAW FILLED RECTANGLE NOW");
	}

	// Main elements of the application
	DrawGrid();
	RenderShapes();

	// Various preview features
	switch (previewShape) {
	case ShapeType::LINE:
		//DrawLine(previewLineStart, mouseSnapped_workspace, ctrl_currentLineThickness, color(0));
		LOG_ERROR("DRAW LINE NOW");
		break;
	}
	if (previewPointShown) {
		//DrawLittlePoint(previewPoint, 4);
		LOG_ERROR("DRAW LITTLE POINT NOW");
	}

	// Draw second part of the selection box
	if (selectionBoxActive) {			// Outline in the front
		glm::vec2 p1 = ConvertWorkspaceToScreenCoords(selectionBoxPointA);
		glm::vec2 p2 = ConvertWorkspaceToScreenCoords(selectionBoxPointB);
		//outlinedRectangle(p1, p2, gfx_selectionBoxColor, 1);
		LOG_ERROR("DRAW OUTLINED RECTANGLE NOW");
	}

	Renderer2D::EndScene();
}

void Navigator::OnEvent(Battery::Event* e) {

}






glm::vec2 Navigator::ConvertScreenToWorkspaceCoords(const glm::vec2& v) {
	return (v - panOffset - glm::vec2(applicationPointer->window.GetWidth(), 
		applicationPointer->window.GetHeight()) * 0.5f) / scale;
}

glm::vec2 Navigator::ConvertWorkspaceToScreenCoords(const glm::vec2& v) {
	return panOffset + v * scale + glm::vec2(applicationPointer->window.GetWidth(), 
		applicationPointer->window.GetHeight()) * 0.5f;
}











bool Navigator::IsShapeSelected(ShapeID id) {

	for (size_t i = 0; i < selectedShapes.size(); i++) {
		if (selectedShapes[i] == id) {
			return true;
		}
	}

	return false;
}












void Navigator::DrawGrid() {
	using namespace Battery;

	float thickness = gridLineWidth;
	glm::vec4 color = glm::vec4(gridLineColor, gridLineColor, gridLineColor, 255);

	int w = applicationPointer->window.GetWidth();
	int h = applicationPointer->window.GetHeight();

	// Sub grid lines
	if (scale * snapSize > 3) {
		for (float x = panOffset.x + w / 2; x < w; x += scale * snapSize) {
			Renderer2D::DrawLine({ x, 0 }, { x, h }, thickness, color);
		}
		for (float x = panOffset.x + w / 2; x > 0; x -= scale * snapSize) {
			Renderer2D::DrawLine({ x, 0 }, { x, h }, thickness, color);
		}
		for (float y = panOffset.y + h / 2; y < h; y += scale * snapSize) {
			Renderer2D::DrawLine({ 0, y }, { w, y }, thickness, color);
		}
		for (float y = panOffset.y + h / 2; y > 0; y -= scale * snapSize) {
			Renderer2D::DrawLine({ 0, y }, { w, y }, thickness, color);
		}
	}
}

void Navigator::RenderShapes() {
	/*
	for (::Layer* layer : file.GetLayersReverse()) {
		for (::Shape* shape : layer->GetShapes()) {

			switch (shape->type) {

			case ShapeType::LINE:
				if (layer->layerID == file.GetActiveLayerID()) {

					// Shape is selected
					if (IsShapeSelected(shape->shapeID)) {
						if (shape->shapeID == lastHoveredShape) {	// Shape is selected and hovered
							DrawLine(shape->p1, shape->p2, shape->thickness, 
								(hoveredLineColor + selectedLineColor) / 2.f);
						}
						else {
							DrawLine(shape->p1, shape->p2, shape->thickness, selectedLineColor);
						}
					}
					else { // Shape is simply hovered
						if (shape->shapeID == lastHoveredShape) {
							DrawLine(shape->p1, shape->p2, shape->thickness, hoveredLineColor);
						}
						else {
							DrawLine(shape->p1, shape->p2, shape->thickness, normalLineColor);
						}
					}
				}
				else {
					drawLine(shape->p1, shape->p2, shape->thickness, gfx_disabledLineColor);
				}
				break;

			case SHAPE_CIRCLE:
				break;

			default:
				break;
			}
		}
	}*/
}
