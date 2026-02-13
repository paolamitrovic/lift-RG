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

void Floor::setup(float width, float depth, float y, bool isCeiling) {
    // Pod je pravougaonik u XZ ravni na visini y
    // Za plafon (isCeiling=true): obrnuta normala (0, -1, 0) i obrnut redosled vertexa
    // Vertices: pozicija (x,y,z), boja (r,g,b,a), texCoord (s,t), normala (nx,ny,nz)
    float normalY = isCeiling ? -1.0f : 1.0f; // Obrnuta normala za plafon
    
    float vertices[4 * 12]; // 4 vertexa, svaki sa 12 komponenti
    if (isCeiling) {
        // Plafon - obrnut redosled vertexa i obrnuta normala
        // Gornji levi (za plafon ovo je donji levi)
        vertices[0] = -width/2; vertices[1] = y; vertices[2] = depth/2;
        vertices[3] = 1.0f; vertices[4] = 1.0f; vertices[5] = 1.0f; vertices[6] = 1.0f;
        vertices[7] = 0.0f; vertices[8] = 0.0f;
        vertices[9] = 0.0f; vertices[10] = normalY; vertices[11] = 0.0f;
        
        // Gornji desni
        vertices[12] = width/2; vertices[13] = y; vertices[14] = depth/2;
        vertices[15] = 1.0f; vertices[16] = 1.0f; vertices[17] = 1.0f; vertices[18] = 1.0f;
        vertices[19] = 1.0f; vertices[20] = 0.0f;
        vertices[21] = 0.0f; vertices[22] = normalY; vertices[23] = 0.0f;
        
        // Donji desni
        vertices[24] = width/2; vertices[25] = y; vertices[26] = -depth/2;
        vertices[27] = 1.0f; vertices[28] = 1.0f; vertices[29] = 1.0f; vertices[30] = 1.0f;
        vertices[31] = 1.0f; vertices[32] = 1.0f;
        vertices[33] = 0.0f; vertices[34] = normalY; vertices[35] = 0.0f;
        
        // Donji levi
        vertices[36] = -width/2; vertices[37] = y; vertices[38] = -depth/2;
        vertices[39] = 1.0f; vertices[40] = 1.0f; vertices[41] = 1.0f; vertices[42] = 1.0f;
        vertices[43] = 0.0f; vertices[44] = 1.0f;
        vertices[45] = 0.0f; vertices[46] = normalY; vertices[47] = 0.0f;
    } else {
        // Pod - normalan redosled
        // Donji levi
        vertices[0] = -width/2; vertices[1] = y; vertices[2] = -depth/2;
        vertices[3] = 1.0f; vertices[4] = 1.0f; vertices[5] = 1.0f; vertices[6] = 1.0f;
        vertices[7] = 0.0f; vertices[8] = 0.0f;
        vertices[9] = 0.0f; vertices[10] = normalY; vertices[11] = 0.0f;
        
        // Donji desni
        vertices[12] = width/2; vertices[13] = y; vertices[14] = -depth/2;
        vertices[15] = 1.0f; vertices[16] = 1.0f; vertices[17] = 1.0f; vertices[18] = 1.0f;
        vertices[19] = 1.0f; vertices[20] = 0.0f;
        vertices[21] = 0.0f; vertices[22] = normalY; vertices[23] = 0.0f;
        
        // Gornji desni
        vertices[24] = width/2; vertices[25] = y; vertices[26] = depth/2;
        vertices[27] = 1.0f; vertices[28] = 1.0f; vertices[29] = 1.0f; vertices[30] = 1.0f;
        vertices[31] = 1.0f; vertices[32] = 1.0f;
        vertices[33] = 0.0f; vertices[34] = normalY; vertices[35] = 0.0f;
        
        // Gornji levi
        vertices[36] = -width/2; vertices[37] = y; vertices[38] = depth/2;
        vertices[39] = 1.0f; vertices[40] = 1.0f; vertices[41] = 1.0f; vertices[42] = 1.0f;
        vertices[43] = 0.0f; vertices[44] = 1.0f;
        vertices[45] = 0.0f; vertices[46] = normalY; vertices[47] = 0.0f;
    }
    
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
