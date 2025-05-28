
#ifndef GL_HPP
#define GL_HPP

#define TINYSTD_USE_CLAY
#include "../GL/glad.h"
#include "../shared/clay.h"

#include "../shared/TinyStd.hpp"

namespace gl {

using ts::Color;
using ts::Vec2f;
using ts::Vec3f;

struct Vertex {
  Vec3f pos;
  Color color;
  Vec2f uv;
};

enum struct DrawType {
  Static = GL_STATIC_DRAW,
  Dynamic = GL_DYNAMIC_DRAW,
};

enum struct Format {
  RGBA = GL_RGBA,
  BGRA = GL_BGRA,
};

template <typename T> void setUniform(GLuint shaderProgram, const char* name, const T& value)
{
  GLuint uniform = glGetUniformLocation(shaderProgram, name);
  if (uniform == -1) {
    ts::print_stderr("Warning: Uniform '{}' was not found", name);
    return;
  }
  if constexpr (ts::is_same<T, int>::value) {
    glUniform1i(uniform, value);
  } else if constexpr (ts::is_same<T, float>::value) {
    glUniform1f(uniform, value);
  } else {
    static_assert(false && false, "That datatype is not supported yet in gl::setUniform");
  }
}

inline void uploadVertexBufferData(GLuint buffer, Vertex* vertices, size_t numberOfVertices, DrawType drawType)
{
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, numberOfVertices * sizeof(Vertex), vertices, (int)drawType);
}

inline void uploadIndexBufferData(GLuint buffer, GLuint* indices, size_t numberOfIndices, DrawType drawType)
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, numberOfIndices * sizeof(GLuint), indices, (int)drawType);
}

inline void setupBuffers()
{
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vec3f)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vec3f) + sizeof(Color)));
  glEnableVertexAttribArray(2);
}

struct Texture {
  GLuint textureId = {};

  static Texture create()
  {
    Texture texture;
    glGenTextures(1, &texture.textureId);
    if (!texture.textureId) {
      ts::panic("Creating GL Texture failed");
    }
    return texture;
  }

  ts::Vec2i getSize()
  {
    int width, height;
    glBindTexture(GL_TEXTURE_2D, textureId);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    return ts::Vec2i(width, height);
  }

  void clear(ts::Vec2i size)
  {
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  void uploadData(ts::Vec2i size, Format format, void* data)
  {
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, (int)format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  void free()
  {
    if (textureId) {
      glDeleteTextures(1, &textureId);
    }
    textureId = 0;
  }
};

} // namespace gl

#endif // GL_HPP