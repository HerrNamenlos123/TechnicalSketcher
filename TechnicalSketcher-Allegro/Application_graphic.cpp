
#include "pch.h"
#include "Application.h"
#include "utils.h"



void Application::renderApplication() {

	background(255);

	// First part of the selection box
	if (drawingSelectionBox) {			// Fill should be in the back
		filledRectangle(convertWorkspaceToScreenCoords(previewLineStart), mouse, gfx_selectionBoxFillColor);
	}

	// Main elements of the application
	drawGrid();
	renderShapes();

	// Various preview features
	if (drawingLine) {
		drawLine(previewLineStart, mouseSnapped_workspace, ctrl_currentLineThickness, color(0));
	}
	if (showPreviewPoint) {
		drawLittlePoint(previewPoint, 4);
	}

	// Second part of the selection box
	if (drawingSelectionBox) {			// Outline in the front
		outlinedRectangle(convertWorkspaceToScreenCoords(previewLineStart), mouse, gfx_selectionBoxColor, 1);
	}
}







void Application::renderShapes() {

	for (LayerID id : layers.getSortedLayerIDsReverse()) {
		Layer& layer = layers.findLayer(id);

		for (Shape& shape : layer.getShapes()) {

			switch (shape.type) {
			case SHAPE_LINE:

				if (layer.layerID == layers.getSelectedLayerID()) {

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

void Application::renderLayerToBitmap(LayerID layer, ALLEGRO_BITMAP* bitmap) {
	std::vector<Shape>& shapes = layers.findLayer(layer).getShapes();

	int sizeX = al_get_bitmap_width(bitmap);
	int sizeY = al_get_bitmap_height(bitmap);

	// If no shapes, just render white
	if (shapes.size() == 0) {
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
ALLEGRO_BITMAP* Application::createLayerPreviewBitmap(LayerID layer, int sizeX, int sizeY) {

	ALLEGRO_BITMAP* bitmap = al_create_bitmap(sizeX, sizeY);
	if (bitmap == nullptr) {
		return nullptr;
	}

	renderLayerToBitmap(layer, bitmap);

	return bitmap;
}

void Application::drawLittlePoint(glm::vec2 pos, float size) {
	glm::vec2 p = convertWorkspaceToScreenCoords(pos);

	outlinedFilledRectangleCentered(p, { size, size }, color(255), color(0), 1);
}

void Application::drawLine(glm::vec2 from, glm::vec2 to, float thickness, glm::vec3 color) {
	glm::vec2 p1 = convertWorkspaceToScreenCoords(from);
	glm::vec2 p2 = convertWorkspaceToScreenCoords(to);

	fancyLine(p1, p2, color, thickness * scale);
}

void Application::drawOutlinedRectangle(glm::vec2 bottomleft, glm::vec2 topright, float outlineThickness, glm::vec3 color) {
	glm::vec2 p1 = convertWorkspaceToScreenCoords(bottomleft);
	glm::vec2 p2 = convertWorkspaceToScreenCoords(topright);

	fancyLine({ p1.x, p1.y }, { p2.x, p1.y }, color, outlineThickness * scale);
	fancyLine({ p2.x, p1.y }, { p2.x, p2.y }, color, outlineThickness * scale);
	fancyLine({ p2.x, p2.y }, { p1.x, p2.y }, color, outlineThickness * scale);
	fancyLine({ p1.x, p2.y }, { p1.x, p1.y }, color, outlineThickness * scale);
}

void Application::drawGrid() {

	float thickness = gridLineWidth;
	float col = gridLineColor;

	// Sub grid lines
	if (scale * snapSize > 3) {
		for (float x = panOffset.x + width / 2; x < width; x += scale * snapSize) {
			line({ x, 0 }, { x, height }, color(col), thickness);
		}
		for (float x = panOffset.x + width / 2; x > 0; x -= scale * snapSize) {
			line({ x, 0 }, { x, height }, color(col), thickness);
		}
		for (float y = panOffset.y + height / 2; y < height; y += scale * snapSize) {
			line({ 0, y }, { width, y }, color(col), thickness);
		}
		for (float y = panOffset.y + height / 2; y > 0; y -= scale * snapSize) {
			line({ 0, y }, { width, y }, color(col), thickness);
		}
	}
}