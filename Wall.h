#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Wall {
    unsigned int VAO, VBO;
    unsigned int texture;
    glm::mat4 modelMatrix;
    
    Wall();
    void setup(float width, float height, float x, float y, float z, float rotationY);
    void draw(unsigned int shader);
};
