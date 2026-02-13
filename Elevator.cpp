#include "Elevator.h"
#include "Util.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Elevator::Elevator() {
    for (int i = 0; i < 6; i++) {
        glGenVertexArrays(1, &VAO[i]);
        glGenBuffers(1, &VBO[i]);
    }
    glGenVertexArrays(1, &VAOdoorLeft);
    glGenBuffers(1, &VBOdoorLeft);
    glGenVertexArrays(1, &VAOdoorRight);
    glGenBuffers(1, &VBOdoorRight);
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
    
    unsigned int stride = (3 + 4 + 2 + 3) * sizeof(float);
    
    // 1. POD LIFTA (Y=0.01, normala gore) - malo iznad poda hodnika da se izbegne z-fighting
    float floorVertices[] = {
        -w/2, 0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         w/2, 0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         w/2, 0.01f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        -w/2, 0.01f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
    };
    
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // 2. PLAFON LIFTA (Y=h-0.01, normala dole) - malo ispod plafona sprata da se vidi
    float ceilingVertices[] = {
        -w/2, h-0.01f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         w/2, h-0.01f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         w/2, h-0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
        -w/2, h-0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
    };
    
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ceilingVertices), ceilingVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // 3. PREDNJI ZID - NE KORISTIMO GA, umesto toga koristimo dve polovine vrata
    // (Prednji zid se ne renderuje, već se renderuju dve polovine vrata)
    
    // LEVA POLOVINA VRATA (od -w/2 do 0)
    float doorLeftVertices[] = {
        -w/2, 0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.0f, 0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.0f, h-0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -w/2, h-0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    };
    
    glBindVertexArray(VAOdoorLeft);
    glBindBuffer(GL_ARRAY_BUFFER, VBOdoorLeft);
    glBufferData(GL_ARRAY_BUFFER, sizeof(doorLeftVertices), doorLeftVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // DESNA POLOVINA VRATA (od 0 do w/2)
    float doorRightVertices[] = {
         0.0f, 0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         w/2, 0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         w/2, h-0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
         0.0f, h-0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    };
    
    glBindVertexArray(VAOdoorRight);
    glBindBuffer(GL_ARRAY_BUFFER, VBOdoorRight);
    glBufferData(GL_ARRAY_BUFFER, sizeof(doorRightVertices), doorRightVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // 4. ZADNJI ZID (Z=+d/2, normala ka -Z) - počinje od 0.01 da se spoji sa podom, ide do h-0.01 da se spoji sa plafonom
    float backWallVertices[] = {
         w/2, 0.01f, d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        -w/2, 0.01f, d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        -w/2, h-0.01f, d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
         w/2, h-0.01f, d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
    };
    
    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backWallVertices), backWallVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // 5. LEVI ZID (X=-w/2, normala ka +X) - počinje od 0.01 da se spoji sa podom, ide do h-0.01 da se spoji sa plafonom
    float leftWallVertices[] = {
        -w/2, 0.01f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        -w/2, 0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        -w/2, h-0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        -w/2, h-0.01f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
    };
    
    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftWallVertices), leftWallVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // 6. DESNI ZID (X=+w/2, normala ka -X) - počinje od 0.01 da se spoji sa podom, ide do h-0.01 da se spoji sa plafonom
    float rightWallVertices[] = {
         w/2, 0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
         w/2, 0.01f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
         w/2, h-0.01f,  d/2,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
         w/2, h-0.01f, -d/2,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
    };
    
    glBindVertexArray(VAO[5]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightWallVertices), rightWallVertices, GL_STATIC_DRAW);
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
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, posY, z)); // Pod lifta na Y=posY (visina sprata)
}

void Elevator::draw(unsigned int shader, float doorOpenAmount) {
    glUseProgram(shader);
    unsigned int modelLoc = glGetUniformLocation(shader, "uM");
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Isključi face culling da se lift vidi sa obe strane
    bool cullingEnabled = glIsEnabled(GL_CULL_FACE);
    if (cullingEnabled) {
        glDisable(GL_CULL_FACE);
    }
    
    // Omogući depth offset da se izbegne z-fighting i da lift bude iznad ostalih objekata
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f); // Negativan offset da lift bude bliže kameri (iznad)
    
    // Renderuj sve strane OSIM prednjeg zida (index 2)
    for (int i = 0; i < 6; i++) {
        if (i != 2) { // Preskoči prednji zid (renderujemo vrata umesto njega)
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glBindVertexArray(VAO[i]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
    }
    
    // Renderuj VRATA (dve polovine koje se razdvajaju)
    // doorOpenAmount: 0=zatvoreno, 1=potpuno otvoreno
    float doorOffset = doorOpenAmount * width * 0.5f; // Pomeraj za svaku polovinu (maksimalno width/2)
    
    // Leva polovina - pomera se levo
    glm::mat4 leftDoorMatrix = modelMatrix;
    leftDoorMatrix = glm::translate(leftDoorMatrix, glm::vec3(-doorOffset, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(leftDoorMatrix));
    glBindVertexArray(VAOdoorLeft);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Desna polovina - pomera se desno
    glm::mat4 rightDoorMatrix = modelMatrix;
    rightDoorMatrix = glm::translate(rightDoorMatrix, glm::vec3(doorOffset, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rightDoorMatrix));
    glBindVertexArray(VAOdoorRight);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glBindVertexArray(0);
    
    // Isključi depth offset
    glDisable(GL_POLYGON_OFFSET_FILL);
    
    // Vrati prethodno stanje
    if (cullingEnabled) {
        glEnable(GL_CULL_FACE);
    }
}
