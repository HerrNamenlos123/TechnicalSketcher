#pragma once

#include "pch.h"
#include "utils.h"

float getNormalDistanceToLine(glm::vec2 a, glm::vec2 b, glm::vec2 p) {
    float d = dist(a.x, a.y, b.x, b.y);

    if (d == 0)
        return dist(a.x, a.y, p.x, p.y);

    return abs((b.y - a.y) * p.x - (b.x - a.x) * p.y + b.x * a.y - b.y * a.x) / d;
}

float getDistanceAroundLine(glm::vec2 a, glm::vec2 b, glm::vec2 p) {
    glm::vec2 aToB = b - a;
    glm::vec2 aToP = p - a;
    glm::vec2 bToA = a - b;
    glm::vec2 bToP = p - b;

    if (aToB.length() == 0)
        return aToP.length();

    if (glm::dot(aToB, aToP) < 0) {
        return dist(a.x, a.y, p.x, p.y);
    }
    else if (glm::dot(bToA, bToP) < 0) {
        return dist(b.x, b.y, p.x, p.y);
    }
    else {
        return getNormalDistanceToLine(a, b, p);
    }
}