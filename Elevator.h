#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Elevator {
    unsigned int VAO[6];  // 6 strana: pod, plafon, 4 zida
    unsigned int VBO[6];
    unsigned int VAOdoorLeft, VAOdoorRight;  // Dve polovine vrata (leva i desna)
    unsigned int VBOdoorLeft, VBOdoorRight;
    unsigned int texture;
    glm::mat4 modelMatrix;
    float x, y, z;
    float width, depth, height;
    
    Elevator();
    void setup(float w, float d, float h, float posX, float posY, float posZ);
    void draw(unsigned int shader, float doorOpenAmount);  // doorOpenAmount: 0=zatvoreno, 1=potpuno otvoreno
};
