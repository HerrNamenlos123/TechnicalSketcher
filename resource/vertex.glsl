
#version 400 core

attribute vec4 al_pos;
attribute vec4 al_color;

uniform mat4 al_projview_matrix;

varying vec4 color;
varying vec2 screenPos;

void main()
{
   screenPos = al_pos.xy;
   color = al_color;
   gl_Position = al_projview_matrix * al_pos;
}


/*
attribute vec4 al_pos;
attribute vec4 al_color;
attribute vec2 al_texcoord;

uniform mat4 al_projview_matrix;

varying vec2 screenPos;
varying vec2 uv;
varying vec4 color;

void main()
{
   screenPos = al_pos.xy;
   color = al_color;
   uv = al_texcoord * vec2(1, -1);
   gl_Position = al_projview_matrix * al_pos;
}
*/