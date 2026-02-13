// Autori: Nedeljko Tesanovic i Vasilije Markovic
// Opis: 
 
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLM biblioteke
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Util.h"
#include "Floor.h"
#include "Wall.h"
#include "Elevator.h"
#include "model.hpp"

bool useTex = true; // Uključimo teksture po defaultu
bool transparent = false;

// Name texture overlay
unsigned int nameTexture;
unsigned int rectShader;
unsigned int modelShader;
unsigned int VAOrect;
unsigned int VBOrect;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        useTex = !useTex;
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        transparent = !transparent;
    }
}

unsigned int preprocessTexture(const char* filepath) {
    unsigned int texture = loadImageToTexture(filepath); // Učitavanje teksture
    glBindTexture(GL_TEXTURE_2D, texture); // Vezujemo se za teksturu kako bismo je podesili

    // Generisanje mipmapa - predefinisani različiti formati za lakše skaliranje po potrebi (npr. da postoji 32 x 32 verzija slike, ali i 16 x 16, 256 x 256...)
    glGenerateMipmap(GL_TEXTURE_2D);

    // Podešavanje strategija za wrap-ovanje - šta da radi kada se dimenzije teksture i poligona ne poklapaju
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S - tekseli po x-osi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T - tekseli po y-osi

    // Podešavanje algoritma za smanjivanje i povećavanje rezolucije: nearest - bira najbliži piksel, linear - usrednjava okolne piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

bool firstMouse = true;
float lastX, lastY;
float yaw = -90.0f, pitch = 0.0f; // yaw -90: kamera gleda u pravcu z ose; pitch = 0: kamera gleda vodoravno
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0); // at-vektor je inicijalno u pravcu z ose
glm::vec3 cameraPos = glm::vec3(0.0, 1.5, 5.0); // Početna pozicija kamere (visina čoveka)
float cameraSpeed = 0.05f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}
float fov = 45.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

int main(void)
{
    if (!glfwInit())
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth, wHeight;
    const char wTitle[] = "Vezbe 7";
    
    // Fullscreen mode
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    wWidth = mode->width;
    wHeight = mode->height;
    lastX = wWidth / 2.0f;
    lastY = wHeight / 2.0f;
    window = glfwCreateWindow(wWidth, wHeight, wTitle, monitor, NULL);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }
    
    glfwMakeContextCurrent(window);

    
    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++
    
    // Load name texture for overlay
    nameTexture = preprocessTexture("res/name.png");
    
    // Load shader for 3D models (with normals)
    modelShader = createShader("model.vert", "model.frag");
    
    // Create shader for 2D overlay
    rectShader = createShader("rect.vert", "rect.frag");
    glUseProgram(rectShader);
    glUniform1i(glGetUniformLocation(rectShader, "uTex0"), 0);
    
    // Create VAO and VBO for name overlay rectangle
    float verticesRect[] = {
         0.7f, -1.0f,  0.0f, 0.0f, // donji levi
         1.0f, -1.0f,  1.0f, 0.0f, // donji desni
         1.0f, -0.85f,  1.0f, 1.0f, // gornji desni
         0.7f, -0.85f,  0.0f, 1.0f  // gornji levi
    };
    
    glGenVertexArrays(1, &VAOrect);
    glGenBuffers(1, &VBOrect);
    glBindVertexArray(VAOrect);
    glBindBuffer(GL_ARRAY_BUFFER, VBOrect);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRect), verticesRect, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    glUseProgram(unifiedShader);
    glUniform1i(glGetUniformLocation(unifiedShader, "uTex"), 0);
    
    glEnable(GL_DEPTH_TEST);
    
    // Učitaj teksture
    unsigned int floorTexture = preprocessTexture("textures floor/diagonal_parquet_diff_2k.jpg");
    unsigned int wallTexture = preprocessTexture("textures wall/rock_face_03_rough_2k.jpg");
    unsigned int elevatorTexture = preprocessTexture("textures elevator/blue_metal_plate_rough_2k.jpg");
    
    // Dimenzije sprata - uža prostorija kao hodnik
    float floorWidth = 3.0f;   // Uža širina (kao hodnik)
    float floorDepth = 10.0f;  // Dužina hodnika
    float wallHeight = 3.0f;
    
    // Kreiraj sprat (pod)
    Floor floor;
    floor.setup(floorWidth, floorDepth, 0.0f); // širina, dubina, visina Y
    floor.texture = floorTexture;
    
    // Kreiraj zidove (4 zida okrenuta ka unutra)
    // Zidovi se postavljaju tako da se donja ivica (Y=0) spoji sa podom
    // y parametar u setup() se ne koristi za Y translaciju (zid uvek počinje na Y=0)
    Wall walls[4];
    // Prednji zid (gleda ka -Z) - na poziciji z = -floorDepth/2
    walls[0].setup(floorWidth, wallHeight, 0.0f, 0.0f, -floorDepth/2.0f, 0.0f);
    walls[0].texture = wallTexture;
    // Zadnji zid (gleda ka +Z) - na poziciji z = floorDepth/2
    walls[1].setup(floorWidth, wallHeight, 0.0f, 0.0f, floorDepth/2.0f, 180.0f);
    walls[1].texture = wallTexture;
    // Levi zid (gleda ka +X) - na poziciji x = -floorWidth/2
    walls[2].setup(floorDepth, wallHeight, -floorWidth/2.0f, 0.0f, 0.0f, 90.0f);
    walls[2].texture = wallTexture;
    // Desni zid (gleda ka -X) - na poziciji x = floorWidth/2
    walls[3].setup(floorDepth, wallHeight, floorWidth/2.0f, 0.0f, 0.0f, -90.0f);
    walls[3].texture = wallTexture;
    
    // Kreiraj plafon (gornji zid)
    Floor ceiling;
    ceiling.setup(floorWidth, floorDepth, wallHeight);
    ceiling.texture = wallTexture;
    
    // Kreiraj lift - kvadar na kraju hodnika (suprotno od biljke)
    // Lift je uzak ali realan - dubina 1.5m, popunjava od zida do zida, od poda do plafona
    // Lift je malo unutar zidova da se izbegne z-fighting
    float elevatorDepth = 1.5f;  // Dubina lifta
    float elevatorZ = floorDepth/2.0f - elevatorDepth/2.0f;  // Pozicija lifta na kraju hodnika
    float elevatorWidth = floorWidth - 0.02f;  // Malo uža širina da se izbegne z-fighting sa zidovima
    
    Elevator elevator;
    elevator.setup(elevatorWidth, elevatorDepth, wallHeight, 0.0f, 0.0f, elevatorZ);
    elevator.texture = elevatorTexture;
    
    // Učitaj biljku - u jednom uglu sprata
    Model plant("plant 1/uploads_files_4769167_Flower.obj");
    glm::mat4 plantModel = glm::mat4(1.0f);
    // Pozicija biljke: u uglu (levo, napred) - malo unutar zidova
    plantModel = glm::translate(plantModel, glm::vec3(-floorWidth/2.0f + 0.3f, 0.0f, -floorDepth/2.0f + 0.3f));
    plantModel = glm::scale(plantModel, glm::vec3(3.0f, 3.0f, 3.0f)); // Povećano skaliranje da se vidi
    
    // Granice kretanja kamere (čoveka)
    float minX = -floorWidth/2.0f + 0.2f;  // Leva granica (malo unutar zida)
    float maxX = floorWidth/2.0f - 0.2f;   // Desna granica (malo unutar zida)
    float minZ = -floorDepth/2.0f + 0.2f;  // Prednja granica
    float maxZ = floorDepth/2.0f - 0.2f;   // Zadnja granica
    float cameraY = 1.5f;  // Visina kamere (čoveka)
    

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int modelLoc = glGetUniformLocation(unifiedShader, "uM");
    unsigned int viewLoc = glGetUniformLocation(unifiedShader, "uV");
    unsigned int projectionLoc = glGetUniformLocation(unifiedShader, "uP");
    
    glm::mat4 view;
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
    glm::mat4 projectionP = glm::perspective(glm::radians(fov), (float)wWidth / (float)wHeight, 0.1f, 100.0f);

    glClearColor(0.2, 0.2, 0.3, 1.0); // Tamnija pozadina
    
    // Početna pozicija kamere (čoveka) - na spratu, malo levo od centra
    cameraPos = glm::vec3(-2.0f, cameraY, 0.0f);

    while (!glfwWindowShouldClose(window))
    {
        double startTime = glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        //Testiranje dubine
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            glEnable(GL_DEPTH_TEST); //Ukljucivanje testiranja Z bafera
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            glDisable(GL_DEPTH_TEST);
        }

        //Odstranjivanje lica (Prethodno smo podesili koje lice uklanjamo sa glCullFace)
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            glEnable(GL_CULL_FACE);
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            glDisable(GL_CULL_FACE);
        }

        // WASD kretanje kamere (čoveka) sa ograničenjima
        glm::vec3 newCameraPos = cameraPos;
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            newCameraPos += cameraSpeed * glm::normalize(glm::vec3(cameraFront.x, 0, cameraFront.z));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            newCameraPos -= cameraSpeed * glm::normalize(glm::vec3(cameraFront.x, 0, cameraFront.z));
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            newCameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            newCameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
        }
        
        // Ograniči kretanje unutar granica sprata
        newCameraPos.x = glm::clamp(newCameraPos.x, minX, maxX);
        newCameraPos.z = glm::clamp(newCameraPos.z, minZ, maxZ);
        newCameraPos.y = cameraY; // Fiksna visina
        cameraPos = newCameraPos;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(unifiedShader);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
        
        // Renderuj pod (sa teksturom)
        glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), useTex);
        glUniform1i(glGetUniformLocation(unifiedShader, "transparent"), transparent);
        floor.draw(unifiedShader);
        
        // Renderuj zidove (sa teksturom) - sve 4 zida
        for (int i = 0; i < 4; i++) {
            walls[i].draw(unifiedShader);
        }
        
        // Renderuj plafon sprata
        ceiling.draw(unifiedShader);
        
        // Renderuj lift POSLE plafona i zidova, ali sa negativnim depth offset-om da bude iznad
        elevator.draw(unifiedShader);
        
        // Renderuj biljku sa model shader-om (koristi normalu)
        glUseProgram(modelShader);
        glUniformMatrix4fv(glGetUniformLocation(modelShader, "uM"), 1, GL_FALSE, glm::value_ptr(plantModel));
        glUniformMatrix4fv(glGetUniformLocation(modelShader, "uV"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(modelShader, "uP"), 1, GL_FALSE, glm::value_ptr(projectionP));
        glUniform3f(glGetUniformLocation(modelShader, "uLightPos"), 0.0f, 2.0f, 0.0f);
        glUniform3fv(glGetUniformLocation(modelShader, "uViewPos"), 1, glm::value_ptr(cameraPos));
        glUniform3f(glGetUniformLocation(modelShader, "uLightColor"), 1.0f, 1.0f, 1.0f);
        plant.Draw(modelShader);
        glUseProgram(unifiedShader); // Vrati nazad na unified shader

        // Render name texture overlay (always on top)
        glDisable(GL_DEPTH_TEST); // Disable depth testing for overlay
        glUseProgram(rectShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, nameTexture);
        glUniform1f(glGetUniformLocation(rectShader, "uA"), 0.2f); // Semi-transparent (20% opacity)
        glBindVertexArray(VAOrect);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glEnable(GL_DEPTH_TEST); // Re-enable depth testing

        while (glfwGetTime() - startTime < 1.0 / 75.0) {}
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    // Cleanup overlay resources
    glDeleteBuffers(1, &VBOrect);
    glDeleteVertexArrays(1, &VAOrect);
    glDeleteProgram(rectShader);
    glDeleteTextures(1, &nameTexture);
    
    // Cleanup textures
    glDeleteTextures(1, &floorTexture);
    glDeleteTextures(1, &wallTexture);

    glDeleteProgram(unifiedShader);
    glDeleteProgram(modelShader);

    glfwTerminate();
    return 0;
}
