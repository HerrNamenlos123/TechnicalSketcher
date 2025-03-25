
#include "../GL/glad.h"
#include "../shared/format.h"
#include "colors.h"
#include <GL/glext.h>
#include <SDL3/SDL_opengl.h>

const char* vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
})";

const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

GLuint CompileShader(GLenum type, const char* src)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    StackArena<8192> arena;
    char* buf = arena.allocate<char>(logLength);
    glGetShaderInfoLog(shader, logLength, &logLength, buf);
    print("{}Shader failed to compile: {}{}", RED, String::view(buf, logLength), RESET);
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

GLuint CreateShaderProgram()
{
  GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
  if (!vertexShader) {
    return 0;
  }
  GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
  if (!fragmentShader) {
    glDeleteShader(vertexShader);
    return 0;
  }
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    StackArena<8192> arena;
    char* buf = arena.allocate<char>(logLength);
    glGetProgramInfoLog(program, logLength, &logLength, buf);
    print("{}Shader failed to compile: {}{}", RED, String::view(buf, logLength), RESET);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(program);
    return 0;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return program;
}