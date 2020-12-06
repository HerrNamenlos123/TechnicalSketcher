
#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "utils.h"

struct DirectionalLight {
    glm::vec3 direction = {0, 0, 0};

    glm::vec3 color = {1, 1, 1};
    float intensity = 1;

    glm::mat4 lightSpaceMatrix;
    bool active = 0;

    void setUniforms(const ShaderProgram& shader) {
    	shader.uniformVec("directionalLight.direction", direction);
    	shader.uniformVec("directionalLight.color", color);
    	shader.uniformFloat("directionalLight.intensity", intensity);
    	shader.uniformInt("directionalLight.isActive", active);
    	shader.uniformMat("directionalLight.lightSpaceMatrix", lightSpaceMatrix);
    }
};

#endif // DIRECTIONAL_LIGHT_H
