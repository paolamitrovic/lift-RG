#ifndef MODEL_H
#define MODEL_H
#include "stb_image.h"

#include <GL/glew.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    map<string, Texture> material_textures; // Map materijala na teksture (po imenu)
    vector<Texture> material_textures_by_index; // Teksture po redosledu u MTL fajlu (po indeksu)
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;
    unsigned int mesh_counter; // Brojač mesh-ova za dodelu tekstura

    // constructor, expects a filepath to a 3D model.
    Model(string const& path, bool gamma = false) : gammaCorrection(gamma), mesh_counter(0)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(unsigned int shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    // Ručno učitaj teksture iz MTL fajla
    void loadTexturesFromMTL(string const& objPath, string const& dir)
    {
        // Pronađi MTL fajl
        string mtlPath = objPath;
        size_t lastDot = mtlPath.find_last_of('.');
        if (lastDot != string::npos) {
            mtlPath = mtlPath.substr(0, lastDot) + ".mtl";
        }
        
        // Proveri da li MTL fajl postoji
        ifstream mtlFile(mtlPath);
        if (!mtlFile.is_open()) {
            return;
        }
        
        string line;
        string currentMaterial = "";
        while (getline(mtlFile, line)) {
            if (line.find("newmtl") != string::npos) {
                istringstream iss(line);
                string prefix;
                iss >> prefix >> currentMaterial;
            }
            else if (line.find("map_Kd") != string::npos && !currentMaterial.empty()) {
                istringstream iss(line);
                string prefix, texPath;
                iss >> prefix >> texPath;
                
                // Učitaj teksturu
                Texture texture;
                texture.id = TextureFromFile(texPath.c_str(), dir);
                if (texture.id != 0) {
                    texture.type = "uDiffMap";
                    texture.path = texPath;
                    textures_loaded.push_back(texture);
                    material_textures[currentMaterial] = texture; // Mapiraj materijal na teksturu (po imenu)
                    material_textures_by_index.push_back(texture); // Dodaj i po indeksu
                }
            }
        }
        mtlFile.close();
    }

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));
        if (directory == path) {
            directory = path.substr(0, path.find_last_of('\\'));
        }

        // Ručno učitaj teksture iz MTL fajla ako ASSIMP ne učitava
        loadTexturesFromMTL(path, directory);

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "uDiffMap");
        // Ako ASSIMP nije učitao teksture, koristi ručno učitane za ovaj materijal
        if (diffuseMaps.empty() && !material_textures_by_index.empty()) {
            // Koristi teksturu po redosledu mesh-ova (jer ASSIMP ne učitava sve materijale pravilno)
            unsigned int textureIndex = mesh_counter % material_textures_by_index.size();
            diffuseMaps.push_back(material_textures_by_index[textureIndex]);
            mesh_counter++; // Povećaj brojač za sledeći mesh
        }
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "uSpecMap");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                if (texture.id != 0) {  // Samo dodaj ako je tekstura uspešno učitana
                    texture.type = typeName;
                    texture.path = str.C_Str();
                    textures.push_back(texture);
                    textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
                }
            }
        }
        return textures;
    }
};



unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    
    // Ako je apsolutna putanja (C:\ ili /), izvuci samo ime fajla
    if (filename.find(':') != string::npos || filename[0] == '/') {
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != string::npos) {
            filename = filename.substr(lastSlash + 1);
        }
    }
    
    // Pokušaj prvo u textures folderu, pa u glavnom folderu modela
    string fullPath = "";
    if (!directory.empty()) {
        // Prvo pokušaj u textures podfolderu
        string texturesPath = directory + "/textures/" + filename;
        ifstream testFile1(texturesPath);
        if (testFile1.good()) {
            testFile1.close();
            fullPath = texturesPath;
        } else {
            // Pokušaj direktno u folderu modela
            string modelPath = directory + '/' + filename;
            ifstream testFile2(modelPath);
            if (testFile2.good()) {
                testFile2.close();
                fullPath = modelPath;
            } else {
                // Ako ništa ne radi, probaj samo ime fajla
                fullPath = filename;
            }
        }
    } else {
        fullPath = filename;
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        stbi_image_free(data);
        return 0; // Vrati 0 ako tekstura nije učitana
    }

    return textureID;
}
#endif
