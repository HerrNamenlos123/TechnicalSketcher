
#ifndef MODEL_H
#define MODEL_H

#include "glFunctions.h"
#include "Array.h"
#include "ObjLoader.h"

class Renderer {
public:
    Array<float> vertices;
    Array<GLuint> indices;

    std::vector<Vertex> vert;
    std::vector<Face_t> face;

    Material_t material;

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    bool loaded = false;

    GLuint textureID = 0;
    bool textureValid = false;

    Renderer() {
    }

    Renderer(Mesh_t mesh) {
        load(mesh);
    }

    ~Renderer() {
        unload();
    }

    void texture(std::string str) {

        if(!fileExists(str)) {
            if(str != "")
                std::cout << "Invalid texture file: " << str << " does not exist" << std::endl;

            return;
        }
        
        textureID = makeTexture(str);
        textureValid = true;
    }

    void texture(const Image& img) {
        
        textureID = makeTexture(img);
        textureValid = true;
    }

    void addVertex(const Vertex& v) {
        vert.push_back(v);
    }

    void addVertex(glm::vec3 position, glm::vec2 uv, glm::vec3 normal) {
        Vertex v(position, uv, normal);
        vert.push_back(v);
    }

    void addVertex(float x, float y, float z, float uv_x, float uv_y, float n_x, float n_y, float n_z) {
        Vertex v(glm::vec3(x, y, z), glm::vec2(uv_x, uv_y), glm::vec3(n_x, n_y, n_z));
        vert.push_back(v);
    }

    void addFace(unsigned int v1, unsigned int v2, unsigned int v3) {
        Face_t f(v1, v2, v3);
        face.push_back(f);
    }

    void load(Mesh_t mesh) {
        for (size_t v = 0; v < mesh.vertices.size(); v++) {
            addVertex(mesh.vertices[v]);
        }
    
        for(size_t i = 0; i < mesh.indices.size(); i += 3) {
            addFace(mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]);
        }
    
        material = mesh.material;
        texture(material.diffuseTexture);
        load();
    }

    void load() {
        unload();

        vertices.allocate(vert.size() * 8);
        for(size_t i = 0; i < vert.size(); i++) {
            Vertex v = vert[i];
            vertices[8 * i    ] = v.position.x;
            vertices[8 * i + 1] = v.position.y;
            vertices[8 * i + 2] = v.position.z;
            vertices[8 * i + 3] = v.uv.x;
            vertices[8 * i + 4] = v.uv.y;
            vertices[8 * i + 5] = v.normal.x;
            vertices[8 * i + 6] = v.normal.y;
            vertices[8 * i + 7] = v.normal.z;
        }

        indices.allocate(face.size() * 3);
        for(size_t i = 0; i < face.size(); i++) {
            indices[3 * i    ] = face[i].vertex1;
            indices[3 * i + 1] = face[i].vertex2;
            indices[3 * i + 2] = face[i].vertex3;
        }

        vert.clear();
        face.clear();

        GLCall(glGenVertexArrays(1, &VAO));
        GLCall(glGenBuffers(1, &VBO));
        GLCall(glGenBuffers(1, &EBO));
        GLCall(glBindVertexArray(VAO));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.raw(), GL_STATIC_DRAW));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
        GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.raw(), GL_STATIC_DRAW));

        setVertexAttributes(0, 3, 8, 0, vertices.size(), vertices.raw());  // Position
        setVertexAttributes(1, 2, 8, 3, vertices.size(), vertices.raw());  // UV Coords
        setVertexAttributes(2, 3, 8, 5, vertices.size(), vertices.raw());  // Normal

        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

        loaded = true;
    }

    void unload() {
        if(loaded) {
            deleteBuffers();
            deleteTexture();
            vertices.deallocate();
            indices.deallocate();
        }

        loaded = false;
    }

    void render(GLuint shader) {
        if(!loaded)
            return;

        GLCall(glBindVertexArray(VAO));

        if(textureValid) {
            GLCall(glActiveTexture(GL_TEXTURE0));
            GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
        }

        GLCall(glUniform3f(glGetUniformLocation(shader, "material.ambientColor"), material.ambientColor.x, material.ambientColor.y, material.ambientColor.z));
        GLCall(glUniform3f(glGetUniformLocation(shader, "material.diffuseColor"), material.diffuseColor.x, material.diffuseColor.y, material.diffuseColor.z));
        GLCall(glUniform3f(glGetUniformLocation(shader, "material.specularColor"), material.specularColor.x, material.specularColor.y, material.specularColor.z));
        GLCall(glUniform1f(glGetUniformLocation(shader, "material.shininess"), material.shininess));

        GLCall(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0));
    }

    void deleteBuffers() {
        glDeleteBuffers(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void deleteTexture() {
        if(textureValid) {
            glDeleteTextures(1, &textureID);
        }

        textureValid = false;
        textureID = 0;
    }
};

class Model {
public:
    Array<Renderer> renderers;
    bool loaded = false;

    glm::vec3 position = {0, 0, 0};
    glm::vec2 direction = {0, 0};
    glm::vec3 scaleFactor = {1, 1, 1};
    glm::mat4 standardRotation = glm::mat4(1.0);
    glm::mat4 additionalRotation = glm::mat4(1.0);

	Model() {
	}

	~Model() {
	}

    void setPosition(glm::vec3 v) {
        position = v;
    }

    void setPosition(float x, float y, float z) {
        position = glm::vec3(x, y, z);
    }

    void setDirection(float x, float y) {
        direction = glm::vec2(x, y);
    }

    void scale(glm::vec3 v) {
        scaleFactor = v;
    }

    void scale(float x, float y, float z) {
        scaleFactor = glm::vec3(x, y, z);
    }

    void scale(float s) {
        scaleFactor = glm::vec3(s, s, s);
    }

    void clearStandardRotation() {
        standardRotation = glm::mat4(1.0);
    }

    void clearAdditionalRotation() {
        additionalRotation = glm::mat4(1.0);
    }

    void rotateXStandard(float angle) {
        standardRotation = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(1.0, 0.0, 0.0)) * standardRotation;
    }

    void rotateYStandard(float angle) {
        standardRotation = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(0.0, 1.0, 0.0)) * standardRotation;
    }

    void rotateZStandard(float angle) {
        standardRotation = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(0.0, 0.0, 1.0)) * standardRotation;
    }

    void rotateAroundAxisStandard(glm::vec3 axis, float angle) {
        standardRotation = glm::rotate(glm::mat4(1.0), glm::radians(angle), axis) * standardRotation;
    }

    void rotateX(float angle) {
        additionalRotation = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(1.0, 0.0, 0.0)) * additionalRotation;
    }

    void rotateY(float angle) {
        additionalRotation = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(0.0, 1.0, 0.0)) * additionalRotation;
    }

    void rotateZ(float angle) {
        additionalRotation = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(0.0, 0.0, 1.0)) * additionalRotation;
    }

    void rotateAroundAxis(glm::vec3 axis, float angle) {
        additionalRotation = glm::rotate(glm::mat4(1.0), glm::radians(angle), axis) * additionalRotation;
    }

    void pointDirection(glm::vec3 referenceAxis, glm::vec3 targetDir) {
        clearAdditionalRotation();
        glm::vec3 normalAxis = cross(referenceAxis, targetDir);

        double shortestAngle = angleBetweenVectors(referenceAxis, targetDir);
        if (shortestAngle != 0 && length(normalAxis) != 0) {
            rotateAroundAxis(normalAxis, static_cast<float>(shortestAngle));
        }
    }

    void load(std::string name) {

        std::vector<Mesh_t> meshes;

        timestamp_t start = millis();
        std::cout << "Loading OBJ Model: " << name << std::endl;

        if(loadOBJ(name, &meshes)) {

            renderers.allocate(meshes.size());

            for(size_t mesh = 0; mesh < meshes.size(); mesh++) {

                renderers[mesh].load(meshes[mesh]);

            }

        }

        std::cout << "Total loading time: " << millis() - start << std::endl << std::endl;

        loaded = true;
    }

    void unload() const {
        for(size_t i = 0; i < renderers.size(); i++) {
            renderers[i].unload();
        }
    }

    void load(Mesh_t mesh) {

        renderers.allocate(1);
        renderers[0].load(mesh);

        loaded = true;
    }

    void load(Mesh_t mesh, const Image& texture) {

        renderers.allocate(1);
        renderers[0].texture(texture);
        renderers[0].load(mesh);

        loaded = true;
    }

	void render(GLuint shader) {
        if(!loaded)
            return;

        glm::mat4 modelMatrix = glm::mat4(1.0);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, position.z));

        modelMatrix *= standardRotation;
        modelMatrix *= additionalRotation;
        modelMatrix = glm::rotate(modelMatrix, glm::radians(direction.x), glm::vec3(0.0, 1.0, 0.0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-direction.y), glm::vec3(1.0, 0.0, 0.0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor.x, scaleFactor.y, scaleFactor.z));
        glUniformMatrix4fv(glGetUniformLocation(shader, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);

        for(size_t i = 0; i < renderers.size(); i++) {
            renderers[i].render(shader);
        }
	}

    void operator=(const Model& obj) {

        if(obj.loaded) {
            throw std::logic_error("Hey!! You're copying a loaded Model class! Stop that!!");
        }

    }

};

#endif // MODEL_H
