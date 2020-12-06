#pragma once

#include "glFunctions.h"
#include "gl/SimpleColoredRenderer.h"

enum LINECAP {
	SQUARE,
	ROUND
};

enum RECTMODE {
	CENTER,
	CORNER,
	CORNERS
};

class PrimitiveRenderer {

	SimpleColoredRenderer renderer;

	int* windowWidth = nullptr;
	int* windowHeight = nullptr;
	bool loaded = false;
	
	float _strokeWeight = 3;
	float _resolution = 12;
	glm::vec3 strokeColor = color(0);
	glm::vec3 fillColor = color(255);
	bool useStroke = true;
	bool useFill = true;
	enum RECTMODE _rectMode = CENTER;
	enum LINECAP _lineCap = ROUND;

public:
	PrimitiveRenderer();
	~PrimitiveRenderer();

	void stroke(glm::vec3 col);
	void fill(glm::vec3 col);
	void strokeWeight(float w);
	void noStroke();
	void noFill();
	void rectMode(enum RECTMODE mode);
	void lineCap(enum LINECAP cap);
	void resolution(float r);

	void line(float x1, float y1, float x2, float y2);
	void line(glm::vec2 p1, glm::vec2 p2);
	void rect(float x1, float y1, float x2, float y2);
	void rect(glm::vec2 p1, glm::vec2 p2);
	void circle(float x, float y, float size);
	void circle(glm::vec2 pos, float size);

	void load(int* w, int* h);
	void render();

private:
	glm::vec2 glCoords(glm::vec2 v);

};