
#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "glFunctions.h"
#include <string>

class ShaderProgram {
	bool isValid = false;
	std::string vertexShader;
	std::string fragmentShader;

public:
	GLuint program;

	ShaderProgram();
	ShaderProgram(std::string vs, std::string fs);
	~ShaderProgram();

	bool load(std::string vs, std::string fs);
	bool loadSource(std::string vs, std::string fs);
	bool load();
	void unload();

	GLuint getProgram() const;
	bool valid() const;
	void use() const;
	void release() const;

	void uniformFloat(const char* name, float a) const;
	void uniformFloat(const char* name, float a, float b) const;
	void uniformFloat(const char* name, float a, float b, float c) const;
	void uniformFloat(const char* name, float a, float b, float c, float d) const;
	void uniformInt(const char* name, int a) const;
	void uniformInt(const char* name, int a, int b) const;
	void uniformInt(const char* name, int a, int b, int c) const;
	void uniformInt(const char* name, int a, int b, int c, int d) const;
	void uniformVec(const char* name, glm::vec2 v) const;
	void uniformVec(const char* name, glm::vec3 v) const;
	void uniformVec(const char* name, glm::vec4 v) const;
	void uniformMat(const char* name, glm::mat4 v) const;
};

#endif // SHADER_PROGRAM_H
