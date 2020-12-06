#pragma once

#include "glFunctions.h"
#include "gl/Renderable.h"
#include "gl/ShaderProgram.h"

class SimpleColoredRenderer {

	Renderable renderer;
	ShaderProgram shader;

public:
	SimpleColoredRenderer();
	~SimpleColoredRenderer();

	GLuint addVertex(glm::vec2 position, glm::vec3 color);
	GLuint addFace(GLuint p1, GLuint p2, GLuint p3);
	void clear();
	void load();

	void render();
};
