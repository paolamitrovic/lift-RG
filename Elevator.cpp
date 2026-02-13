#include "Elevator.h"
#include "Util.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Elevator::Elevator() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    texture = 0;
    modelMatrix = glm::mat4(1.0f);
    x = y = z = 0.0f;
    width = depth = height = 1.0f;
}

void Elevator::setup(float w, float d, float h, float posX, float posY, float posZ) {
    width = w;
    depth = d;
    height = h;
    x = posX;
    y = posY;
    z = posZ;
    
    // Lift je kocka (6 strana), ali za sada ćemo napraviti samo pod i zidove
    // Počinjemo sa podom lifta
    float vertices[] = {
        // Pod lifta - donji levi
        -w/2, 0.0f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        // Donji desni
         w/2, 0.0f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        // Gornji desni
         w/2, 0.0f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        // Gornji levi
        -w/2, 0.0f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
    };
    
    unsigned int stride = (3 + 4 + 2 + 3) * sizeof(float);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Postavi model matricu
    // x, y, z su pozicije centra lifta
    // Pod lifta je na Y=0, pa y treba da bude height/2
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, 0.0f, z)); // Pod lifta na Y=0
}

void Elevator::draw(unsigned int shader) {
    glUseProgram(shader);
    unsigned int modelLoc = glGetUniformLocation(shader, "uM");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}
