
#version 400 core

out vec4 FragColor;

uniform vec2 lineP1;
uniform vec2 lineP2;
uniform float lineThickness;
uniform float lineOutline;

varying vec4 color;
varying vec2 uv;
varying vec2 screenPos;

float lineDistance(vec2 P, vec2 L1, vec2 L2) {
    float lower = distance(L1, L2);

    if (lower != 0)
        return abs((L2.x - L1.x)*(L1.y - P.y) - (L1.x - P.x)*(L2.y - L1.y)) / lower;
    else
        return 0;
}

float distanceAroundLine(vec2 P, vec2 L1, vec2 L2) {
    
    vec2 aToB = L2 - L1;
    vec2 aToP = P - L1;
    vec2 bToP = P - L2;

    if (aToB.length() == 0)
        return aToP.length();

    if (dot(aToB, aToP) < 0) {
        return distance(P, L1);
    }
    else if (dot(-aToB, bToP) < 0) {
        return distance(P, L2);
    }

    return lineDistance(P, L1, L2);
}
  
void main()
{
    float dist = distanceAroundLine(screenPos, lineP1, lineP2);

    float a = lineThickness * 0.5;
    float b = lineOutline * 0.5;

    if (a == b && dist > b)
        discard;

    FragColor = vec4(color.xyz, 1 / (a - b) * dist + b / (b - a));
}
