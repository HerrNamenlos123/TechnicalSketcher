
#version 400 core

out vec4 FragColor;

uniform sampler2D al_tex;

varying vec4 color;
varying vec2 uv;
varying vec2 screenPos;
  
void main()
{
    vec3 color = texture(al_tex, uv).xyz;

    if (color.z > 0.5) {
        discard;
        return;
    }

    FragColor = vec4(color, 1);
}

/*
out vec4 FragColor;

uniform sampler2D al_tex;

varying vec2 screenPos;
varying vec2 uv;
varying vec4 color;

void main() {
    
    FragColor = texture(al_tex, uv) + color;

}*/