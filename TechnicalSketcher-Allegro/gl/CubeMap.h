
#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "Window.h"
#include "gl/ShaderProgram.h"
#include <vector>
#include <string>

class CubeMap {
public:
	Window* window;

    GLuint VAO;
    GLuint VBO;
    bool loaded = false;

    ShaderProgram shader;

    GLuint textureID;

public:
    CubeMap();
    ~CubeMap();

    void load(const std::string& texture, Window* w);
    void bind();
    void render(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);

private:
    void deleteBuffers();
    void deleteTexture();
};

#endif // CUBEMAP_H
