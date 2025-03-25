
#include "../GL/glad.h"
#include "../shared/format.h"
#include "colors.h"
#include <GL/glext.h>
#include <SDL3/SDL_opengl.h>

const char* mainVertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4 pixelProjection;

out vec4 vertexColor;

void main() {
  vec4 pos = vec4(aPos, 1.0);
  gl_Position = pixelProjection * pos;
  vertexColor = aColor;
})";

const char* mainFragmentShaderSrc = R"(
#version 330 core

out vec4 FragColor;
in vec4 vertexColor;

void main() {
    FragColor = vertexColor;
})";

const char* lineshapeVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4 pixelProjection;

out vec4 vertexColor;

void main() {
  vec4 pos = vec4(aPos, 1.0);
  gl_Position = pixelProjection * pos;
  vertexColor = aColor;
})";

const char* lineshapeFragmentShader = R"(
#version 330 core

out vec4 FragColor;
in vec4 vertexColor;

void main() {
    FragColor = vertexColor;
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

GLuint CreateShaderProgram(const char* vs, const char* fs)
{
  GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vs);
  if (!vertexShader) {
    return 0;
  }
  GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fs);
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

void setUniformMat4(GLuint shader, const char* name, Mat4 matrix)
{
  GLuint matrixLocation = glGetUniformLocation(shader, name);
  if (matrixLocation == -1) {
    print("{}Uniform not found: {}{}", RED, name, RESET);
    return;
  }
  glUniformMatrix4fv(matrixLocation, 1, GL_TRUE, matrix.data.data());
}
