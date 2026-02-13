#include "Wall.h"
#include "Util.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Wall::Wall() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    texture = 0;
    modelMatrix = glm::mat4(1.0f);
}

void Wall::setup(float width, float height, float x, float y, float z, float rotationY) {
    // Zid je pravougaonik okrenut ka unutra
    // Vertices: pozicija (x,y,z), boja (r,g,b,a), texCoord (s,t), normala (nx,ny,nz)
    // Zid počinje na Y=0 (donja ivica) da se spoji sa podom
    // y parametar je sredina zida po visini, ali zid počinje na 0
    // Zato translacija po Y osi treba da bude 0, a ne y
    float vertices[] = {
        // Donji levi (Y=0 da se spoji sa podom)
        -width/2, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        // Donji desni
         width/2, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        // Gornji desni
         width/2, height, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        // Gornji levi
        -width/2, height, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
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
    
    // Postavi model matricu sa rotacijom i translacijom
    // Translacija po Y osi koristi y parametar da se zid postavi na pravilnu visinu sprata
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z)); // Y=y da se zid postavi na pravilnu visinu sprata
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Wall::draw(unsigned int shader) {
    glUseProgram(shader);
    unsigned int modelLoc = glGetUniformLocation(shader, "uM");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Isključi face culling da se zid vidi sa obe strane
    bool cullingEnabled = glIsEnabled(GL_CULL_FACE);
    if (cullingEnabled) {
        glDisable(GL_CULL_FACE);
    }
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    
    // Vrati prethodno stanje
    if (cullingEnabled) {
        glEnable(GL_CULL_FACE);
    }
}
