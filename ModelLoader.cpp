#include "ModelLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <glm/glm.hpp>

SimpleMesh::SimpleMesh() {
    VAO = VBO = EBO = 0;
    texture = 0;
}

void SimpleMesh::setup() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Format: pozicija(3), boja(4), texCoord(2), normala(3) = 12 float-ova
    unsigned int stride = 12 * sizeof(float);
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

void SimpleMesh::draw(unsigned int shader) {
    glUseProgram(shader);
    
    if (texture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

SimpleMesh loadOBJModel(const char* filepath) {
    SimpleMesh mesh;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<std::string> faceLines;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Failed to open OBJ file: " << filepath << std::endl;
        return mesh;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (prefix == "vt") {
            glm::vec2 tex;
            iss >> tex.x >> tex.y;
            texCoords.push_back(tex);
        }
        else if (prefix == "vn") {
            glm::vec3 norm;
            iss >> norm.x >> norm.y >> norm.z;
            normals.push_back(norm);
        }
        else if (prefix == "f") {
            faceLines.push_back(line);
        }
    }
    file.close();
    
    // Parse faces - koristimo mapu za unique vertekse
    std::map<std::string, unsigned int> vertexMap;
    unsigned int vertexIndex = 0;
    
    for (const auto& faceLine : faceLines) {
        std::istringstream iss(faceLine);
        std::string prefix;
        iss >> prefix; // skip "f"
        
        std::vector<std::string> faceVertices;
        std::string vertex;
        while (iss >> vertex) {
            faceVertices.push_back(vertex);
        }
        
        // OBJ face može biti trougao ili četvorougao, konvertujemo u trouglove
        if (faceVertices.size() >= 3) {
            // Prvi trougao
            for (int i = 0; i < 3; i++) {
                std::string vKey = faceVertices[i];
                if (vertexMap.find(vKey) == vertexMap.end()) {
                    // Novi vertex - parsiraj ga
                    std::istringstream vss(vKey);
                    std::string posStr, texStr, normStr;
                    
                    std::getline(vss, posStr, '/');
                    std::getline(vss, texStr, '/');
                    std::getline(vss, normStr, '/');
                    
                    int posIdx = std::stoi(posStr) - 1;
                    int texIdx = texStr.empty() ? -1 : std::stoi(texStr) - 1;
                    int normIdx = normStr.empty() ? -1 : std::stoi(normStr) - 1;
                    
                    if (posIdx >= 0 && posIdx < positions.size()) {
                        glm::vec3 pos = positions[posIdx];
                        mesh.vertices.push_back(pos.x);
                        mesh.vertices.push_back(pos.y);
                        mesh.vertices.push_back(pos.z);
                        
                        // Boja - zelena za biljku
                        mesh.vertices.push_back(0.2f);  // R
                        mesh.vertices.push_back(0.8f);  // G
                        mesh.vertices.push_back(0.3f);  // B
                        mesh.vertices.push_back(1.0f);  // A
                        
                        // TexCoord
                        if (texIdx >= 0 && texIdx < texCoords.size()) {
                            glm::vec2 tex = texCoords[texIdx];
                            mesh.vertices.push_back(tex.x);
                            mesh.vertices.push_back(tex.y);
                        } else {
                            mesh.vertices.push_back(0.0f);
                            mesh.vertices.push_back(0.0f);
                        }
                        
                        // Normala
                        if (normIdx >= 0 && normIdx < normals.size()) {
                            glm::vec3 norm = normals[normIdx];
                            mesh.vertices.push_back(norm.x);
                            mesh.vertices.push_back(norm.y);
                            mesh.vertices.push_back(norm.z);
                        } else {
                            mesh.vertices.push_back(0.0f);
                            mesh.vertices.push_back(1.0f);
                            mesh.vertices.push_back(0.0f);
                        }
                        
                        vertexMap[vKey] = vertexIndex++;
                    }
                }
                mesh.indices.push_back(vertexMap[vKey]);
            }
            
            // Drugi trougao ako je četvorougao
            if (faceVertices.size() == 4) {
                std::vector<int> quadIndices = {0, 2, 3};
                for (int idx : quadIndices) {
                    std::string vKey = faceVertices[idx];
                    if (vertexMap.find(vKey) == vertexMap.end()) {
                        std::istringstream vss(vKey);
                        std::string posStr, texStr, normStr;
                        std::getline(vss, posStr, '/');
                        std::getline(vss, texStr, '/');
                        std::getline(vss, normStr, '/');
                        
                        int posIdx = std::stoi(posStr) - 1;
                        int texIdx = texStr.empty() ? -1 : std::stoi(texStr) - 1;
                        int normIdx = normStr.empty() ? -1 : std::stoi(normStr) - 1;
                        
                        if (posIdx >= 0 && posIdx < positions.size()) {
                            glm::vec3 pos = positions[posIdx];
                            mesh.vertices.push_back(pos.x);
                            mesh.vertices.push_back(pos.y);
                            mesh.vertices.push_back(pos.z);
                            mesh.vertices.push_back(1.0f);
                            mesh.vertices.push_back(1.0f);
                            mesh.vertices.push_back(1.0f);
                            mesh.vertices.push_back(1.0f);
                            
                            if (texIdx >= 0 && texIdx < texCoords.size()) {
                                glm::vec2 tex = texCoords[texIdx];
                                mesh.vertices.push_back(tex.x);
                                mesh.vertices.push_back(tex.y);
                            } else {
                                mesh.vertices.push_back(0.0f);
                                mesh.vertices.push_back(0.0f);
                            }
                            
                            if (normIdx >= 0 && normIdx < normals.size()) {
                                glm::vec3 norm = normals[normIdx];
                                mesh.vertices.push_back(norm.x);
                                mesh.vertices.push_back(norm.y);
                                mesh.vertices.push_back(norm.z);
                            } else {
                                mesh.vertices.push_back(0.0f);
                                mesh.vertices.push_back(1.0f);
                                mesh.vertices.push_back(0.0f);
                            }
                            
                            vertexMap[vKey] = vertexIndex++;
                        }
                    }
                    mesh.indices.push_back(vertexMap[vKey]);
                }
            }
        }
    }
    
    mesh.setup();
    return mesh;
}
