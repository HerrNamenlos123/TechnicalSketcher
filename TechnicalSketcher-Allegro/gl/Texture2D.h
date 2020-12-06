
#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include "Engine.h"
#include "gl/ShaderProgram.h"
#include "gl/DepthMap.h"

class TextureRaw {

    bool textureValid = false;

public:
	
    GLuint textureID;

	TextureRaw();
	~TextureRaw();

	bool load(const char* texture, bool flipV = false);
	bool load(const DepthMap& depthMap);

	bool isValid();
	void unload();
};

class Texture2D {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    bool buffersValid = false;

	glm::vec2 size;
    Window* window;

    ShaderProgram shader;

public:
	TextureRaw texture;

	Texture2D();
	~Texture2D();

	bool load(const char* textureFile, float width, float height, Window* wind, bool flipV = false);
	bool load(const DepthMap& depthMap, Window* wind);
	bool load();

	void unload();

	void setSize(float width, float height);
	bool render(float x, float y, float alpha = 255);

private:
	void unloadTexture();
	void unloadBuffers();
};

#endif // TEXTURE_2D_H
