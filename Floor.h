#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Floor {
    unsigned int VAO, VBO;
    unsigned int texture;
    glm::mat4 modelMatrix;
    
    Floor();
    void setup(float width, float depth, float y, bool isCeiling = false);
    void draw(unsigned int shader);
};
