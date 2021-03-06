
#version 400 core

out vec4 FragColor;

uniform vec2 arc_center;
uniform float arc_radius;
uniform float arc_start_angle;
uniform float arc_end_angle;
uniform float arc_thickness;
uniform float arc_falloff;

varying vec4 color;
varying vec2 uv;
varying vec2 screenPos;

float PI = 3.1415926535897;

float arcCircleDistance(vec2 P) {
    float distanceToCenter = distance(P, arc_center);

    return abs(arc_radius - distanceToCenter);
}

float distanceAroundArc(vec2 P) {

    vec2 centerToPos = P - arc_center;
    float fragmentAngle = atan(centerToPos.y, -centerToPos.x) + PI;

    if (arc_start_angle < arc_end_angle) {  // Normal case
        if (fragmentAngle >= arc_start_angle && fragmentAngle <= arc_end_angle) {
            return arcCircleDistance(P);
        }
    }
    else {
        if (fragmentAngle <= arc_end_angle || fragmentAngle >= arc_start_angle) {
            return arcCircleDistance(P);
        }
    }

    vec2 p1 = arc_center + vec2(cos(arc_start_angle), -sin(arc_start_angle)) * arc_radius;
    vec2 p2 = arc_center + vec2(cos(arc_end_angle), -sin(arc_end_angle)) * arc_radius;

    float distance1 = distance(P, p1);
    float distance2 = distance(P, p2);

    return min(distance1, distance2);
}
  
void main()
{
    float dist = distanceAroundArc(screenPos);
    
    float a = arc_thickness / 2 - arc_falloff;
    float b = arc_thickness / 2;

    if (dist > b) {
        discard;
        return;
    }
    
    FragColor = vec4(color.xyz, 1 / (a - b) * dist + b / (b - a));
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