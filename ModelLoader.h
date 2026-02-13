#pragma once
#include <GL/glew.h>
#include <vector>
#include <string>

struct SimpleMesh {
    unsigned int VAO, VBO, EBO;
    unsigned int texture;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    SimpleMesh();
    void setup();
    void draw(unsigned int shader);
};

SimpleMesh loadOBJModel(const char* filepath);
