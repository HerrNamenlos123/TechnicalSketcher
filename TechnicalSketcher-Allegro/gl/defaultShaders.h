
#ifndef DEFAULT_SHADERS_H
#define DEFAULT_SHADERS_H

// =====================================================
//  ===            TEXTURED 2D QUADS                ===
// =====================================================

// A very simple vertex shader passing vertices for displaying 2D textures on quads
static const char* VERTEX_SHADER_TEXTURE_2D_SOURCE =
"#version 400 core\n"
"\n"
"layout (location = 0) in vec4 position;\n"
"layout (location = 1) in vec2 texCoord_;\n"
"\n"
"uniform mat4 transformMatrix;\n"
"uniform float size;\n"
"\n"
"out vec2 texCoord;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = transformMatrix * position;\n"
"   texCoord = texCoord_;\n"
"}\n";


// A very simple fragment shader for rendering 2D textures onto quads
static const char* FRAGMENT_SHADER_TEXTURE_2D_SOURCE =
"#version 400 core\n"
"\n"
"uniform sampler2D textureMap;\n"
"uniform float alpha;\n"
"\n"
"in vec2 texCoord;\n"
"\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 color = texture(textureMap, vec2(texCoord.x, -texCoord.y));\n"
"    color.a = alpha;\n"
"    FragColor = color;\n"
"}\n";







// =====================================================
//  ===                 CUBEMAP                     ===
// =====================================================

// A simple vertex shader for rendering a skybox
static const char* VERTEX_SHADER_SKYBOX_SOURCE =
"#version 400 core\n"
"\n"
"layout (location = 0) in vec3 position;\n"
"\n"
"uniform mat4 projectionMatrix;\n"
"uniform mat4 viewMatrix;\n"
"\n"
"out vec3 texCoord;\n"
"\n"
"vec3 coordConversion(vec3 c) { return vec3(c.x, c.z, -c.y); }\n"
"vec4 coordConversion(vec4 c) { return vec4(c.x, c.z, -c.y, c.w); }\n"
"\n"
"//vec3 coordConversion(vec3 c) { return vec3(c.x, c.y, c.z); }\n"
"//vec4 coordConversion(vec4 c) { return vec4(c.x, c.y, c.z, c.w); }\n"
"\n"
"void main()\n"
"{\n"
"   texCoord = coordConversion(position);\n"
"   vec4 pos = projectionMatrix * coordConversion(mat4(mat3(viewMatrix)) * vec4(position, 1.0));\n"
"   gl_Position = pos.xyww;\n"
"}\n";

// A simple fragment shader for rendering a skybox
static const char* FRAGMENT_SHADER_SKYBOX_SOURCE =
"#version 400 core\n"
"\n"
"in vec3 texCoord;\n"
"\n"
"out vec4 FragColor;\n"
"\n"
"uniform samplerCube skybox;\n"
"\n"
"void main()\n"
"{\n"
"    FragColor = texture(skybox, texCoord);\n"
"}\n";







// =====================================================
//  ===         SIMPLE_COLORED_RENDERER             ===
// =====================================================

static const char* VERTEX_SHADER_SIMPLE_COLORED_SOURCE =
"#version 400 core\n"
"\n"
"layout (location = 0) in vec3 position;     // position\n"
"layout (location = 2) in vec3 colorIn;      // actually normal, used as color here\n"
"\n"
"layout (location = 0) out vec3 colorOut;     // position\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(position.xy, 0, 1);\n"
"   colorOut = colorIn;\n"
"}\n";

static const char* FRAGMENT_SHADER_SIMPLE_COLORED_SOURCE =
"#version 400 core\n"
"\n"
"layout (location = 0) in vec3 color;\n"
"\n"
"layout (location = 0) out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"    FragColor = vec4(color, 1.0);\n"
"}\n";




#endif // DEFAULT_SHADERS_H
