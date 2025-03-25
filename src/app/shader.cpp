
#include "../GL/glad.h"

const char* vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
})";

const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red fill
})";

GLuint CompileShader(GLenum type, const char* src)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);
  return shader;
}

GLuint CreateShaderProgram()
{
  GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
  GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return program;
}