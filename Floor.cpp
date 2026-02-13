#include "Floor.h"
#include "Util.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Floor::Floor() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    texture = 0;
    modelMatrix = glm::mat4(1.0f);
}

void Floor::setup(float width, float depth, float y) {
    // Pod je pravougaonik u XZ ravni na visini y
    // Vertices: pozicija (x,y,z), boja (r,g,b,a), texCoord (s,t), normala (nx,ny,nz)
    float vertices[] = {
        // Donji levi
        -width/2, y, -depth/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        // Donji desni
         width/2, y, -depth/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        // Gornji desni
         width/2, y,  depth/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        // Gornji levi
        -width/2, y,  depth/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
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
}

void Floor::draw(unsigned int shader) {
    glUseProgram(shader);
    unsigned int modelLoc = glGetUniformLocation(shader, "uM");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}
