
#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "glFunctions.h"
#include <array>

// This class encompasses OpenGL buffers and vertex arrays.
// The id must be provided and any shader program must be activated beforehands, 
// likewise any uniforms need to be sent before calling render()
// Any textures also need to be created and bound before the call to render()

class Renderable {

    // Containers for vertices and indices

    std::unique_ptr<float[]> rawVertices = nullptr;
    std::unique_ptr<GLuint[]> rawIndices = nullptr;
    size_t numberOfVertices = 0;
    size_t numberOfIndices = 0;
    bool arraysLoaded = false;

    std::vector<std::array<float, 8>> tempVertices;
    std::vector<std::array<GLuint, 3>> tempIndices;

    // OpenGL Buffers

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    bool generated = false;

public:
	Renderable();
	~Renderable();

    GLuint addVertex(glm::vec3 p, glm::vec2 uv = { 0, 0 }, glm::vec3 n = { 0, 0, 0 } );
    GLuint addFace(GLuint p1, GLuint p2, GLuint p3);
    void clear();
    void load();

    void render();

private:
    void generate();
    void loadArrays();
    void deleteArrays();
    void loadBuffers();

};

#endif // RENDERABLE_H
