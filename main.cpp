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
    
    // Učitaj biljku - uz prednji zid (najdalji od lifta, u koji prvo pogleda čovek), na sredini po X osi
    Model plant("plant 1/uploads_files_4769167_Flower.obj");
    glm::mat4 plantModel = glm::mat4(1.0f);
    // Pozicija biljke: sredina po X osi (0.0), uz prednji zid (-floorDepth/2.0f + malo unutar)
    plantModel = glm::translate(plantModel, glm::vec3(0.0f, 0.0f, -floorDepth/2.0f + 0.3f));
    plantModel = glm::scale(plantModel, glm::vec3(3.0f, 3.0f, 3.0f)); // Povećano skaliranje da se vidi
    
    // Učitaj lampu za lift - na sredini plafona lifta, malo ispod
    Model elevatorLamp("elevator lamp/AM152_063_Lugstar_Premium_LED.obj");
    glm::mat4 lampModel = glm::mat4(1.0f);
    // Pozicija lampe: sredina lifta po X i Z osi, malo ispod plafona lifta
    float lampY = wallHeight - 0.15f; // Malo ispod plafona lifta (plafon je na wallHeight - 0.01)
    lampModel = glm::translate(lampModel, glm::vec3(0.0f, lampY, elevatorZ)); // Sredina lifta
    lampModel = glm::scale(lampModel, glm::vec3(0.03f, 0.03f, 0.03f)); // Smanjeno skaliranje
    
    // Učitaj lampu za plafon sprata - na sredini plafona sprata, malo ispod
    Model floorLamp("elevator lamp/AM152_063_Lugstar_Premium_LED.obj");
    glm::mat4 floorLampModel = glm::mat4(1.0f);
    // Pozicija lampe: sredina sprata po X i Z osi, malo ispod plafona sprata
    floorLampModel = glm::translate(floorLampModel, glm::vec3(0.0f, lampY, 0.0f)); // Sredina sprata
    floorLampModel = glm::scale(floorLampModel, glm::vec3(0.03f, 0.03f, 0.03f)); // Isto skaliranje kao lampa u liftu
    
    // VRATA - animacija (iz 2D projekta)
    float doorOpenAmount = 0.0f;         // Koliko su vrata otvorena (0=zatvoreno, 1=potpuno)
    bool doorOpening = false;            // Da li se vrata otvaraju
    bool doorClosing = false;            // Da li se vrata zatvaraju
    bool doorOpen = false;               // Da li su vrata potpuno otvorena
    double doorTimerStart = 0.0;         // Vreme kada su se vrata otvorila
    bool doorExtended = false;           // Da li su vrata PRODUŽENO otvorena
    
    // OSOBA - da li je u liftu
    bool personHasEnteredElevator = false;   // Da li je čovek ušao u lift
    
    // Podešavanja za testiranje dubine i odstranjivanje naličja (toggle stanja)
    bool depthTestWasPressed = false;   // Da li je taster 1 bio pritisnut u prethodnom frame-u
    bool depthTestDisableWasPressed = false; // Da li je taster 2 bio pritisnut u prethodnom frame-u
    bool cullFaceWasPressed = false;    // Da li je taster 3 bio pritisnut u prethodnom frame-u
    bool cullFaceDisableWasPressed = false; // Da li je taster 4 bio pritisnut u prethodnom frame-u
    
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
    
    // Uniforme za Phong osvetljenje - prvi izvor (lampa na spratu)
    unsigned int viewPosLoc = glGetUniformLocation(unifiedShader, "uViewPos");
    unsigned int lightPosLoc = glGetUniformLocation(unifiedShader, "uLight.pos");
    unsigned int lightALoc = glGetUniformLocation(unifiedShader, "uLight.kA");
    unsigned int lightDLoc = glGetUniformLocation(unifiedShader, "uLight.kD");
    unsigned int lightSLoc = glGetUniformLocation(unifiedShader, "uLight.kS");
    unsigned int lightConstantLoc = glGetUniformLocation(unifiedShader, "uLight.constant");
    unsigned int lightLinearLoc = glGetUniformLocation(unifiedShader, "uLight.linear");
    unsigned int lightQuadraticLoc = glGetUniformLocation(unifiedShader, "uLight.quadratic");
    
    // Uniforme za Phong osvetljenje - drugi izvor (lampa u liftu)
    unsigned int light2PosLoc = glGetUniformLocation(unifiedShader, "uLight2.pos");
    unsigned int light2ALoc = glGetUniformLocation(unifiedShader, "uLight2.kA");
    unsigned int light2DLoc = glGetUniformLocation(unifiedShader, "uLight2.kD");
    unsigned int light2SLoc = glGetUniformLocation(unifiedShader, "uLight2.kS");
    unsigned int light2ConstantLoc = glGetUniformLocation(unifiedShader, "uLight2.constant");
    unsigned int light2LinearLoc = glGetUniformLocation(unifiedShader, "uLight2.linear");
    unsigned int light2QuadraticLoc = glGetUniformLocation(unifiedShader, "uLight2.quadratic");
    
    unsigned int materialShineLoc = glGetUniformLocation(unifiedShader, "uMaterial.shine");
    unsigned int materialALoc = glGetUniformLocation(unifiedShader, "uMaterial.kA");
    unsigned int materialDLoc = glGetUniformLocation(unifiedShader, "uMaterial.kD");
    unsigned int materialSLoc = glGetUniformLocation(unifiedShader, "uMaterial.kS");
    
    // Pozicije lampi (izvori svetlosti)
    glm::vec3 elevatorLightPos = glm::vec3(0.0f, lampY, elevatorZ); // Pozicija lampe u liftu
    glm::vec3 floorLightPos = glm::vec3(0.0f, lampY, 0.0f); // Pozicija lampe na spratu
    
    glm::mat4 view;
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
    glm::mat4 projectionP = glm::perspective(glm::radians(fov), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    
    // Postavi početne vrednosti za osvetljenje (koristićemo kombinaciju oba izvora)
    glUseProgram(unifiedShader);
    // Prvi izvor svetlosti (lampa na spratu)
    glUniform3f(lightALoc, 0.3f, 0.3f, 0.3f); // Ambijentalna komponenta
    glUniform3f(lightDLoc, 2.0f, 2.0f, 2.0f); // Difuzna komponenta (jača da se vidi krug)
    glUniform3f(lightSLoc, 1.0f, 1.0f, 1.0f); // Spekularna komponenta
    // Attenuation za prvi izvor (slabljenje sa udaljenošću) - stvara krug svetlosti
    glUniform1f(lightConstantLoc, 1.0f);
    glUniform1f(lightLinearLoc, 0.14f);
    glUniform1f(lightQuadraticLoc, 0.07f);
    
    // Drugi izvor svetlosti (lampa u liftu) - jača svetlost u liftu
    glUniform3f(light2ALoc, 0.3f, 0.3f, 0.3f); // Ambijentalna komponenta
    glUniform3f(light2DLoc, 2.5f, 2.5f, 2.5f); // Difuzna komponenta (jača u liftu)
    glUniform3f(light2SLoc, 1.0f, 1.0f, 1.0f); // Spekularna komponenta
    // Attenuation za drugi izvor (slabljenje sa udaljenošću) - stvara krug svetlosti u liftu
    glUniform1f(light2ConstantLoc, 1.0f);
    glUniform1f(light2LinearLoc, 0.14f);
    glUniform1f(light2QuadraticLoc, 0.07f);
    
    // Materijal za pod, zidove i lift (neutralan materijal)
    glUniform1f(materialShineLoc, 32.0f); // Uglancanost
    glUniform3f(materialALoc, 1.0f, 1.0f, 1.0f); // Ambijentalna refleksija materijala
    glUniform3f(materialDLoc, 1.0f, 1.0f, 1.0f); // Difuzna refleksija materijala
    glUniform3f(materialSLoc, 0.5f, 0.5f, 0.5f); // Spekularna refleksija materijala

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

        //Testiranje dubine - toggle sa tasterom 1 (uključivanje)
        bool depthTestPressed = (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS);
        if (depthTestPressed && !depthTestWasPressed)
        {
            glEnable(GL_DEPTH_TEST); //Ukljucivanje testiranja Z bafera
        }
        depthTestWasPressed = depthTestPressed;
        
        //Testiranje dubine - toggle sa tasterom 2 (isključivanje)
        bool depthTestDisablePressed = (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS);
        if (depthTestDisablePressed && !depthTestDisableWasPressed)
        {
            glDisable(GL_DEPTH_TEST);
        }
        depthTestDisableWasPressed = depthTestDisablePressed;

        //Odstranjivanje lica - toggle sa tasterom 3 (uključivanje)
        bool cullFacePressed = (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS);
        if (cullFacePressed && !cullFaceWasPressed)
        {
            glEnable(GL_CULL_FACE);
        }
        cullFaceWasPressed = cullFacePressed;
        
        //Odstranjivanje lica - toggle sa tasterom 4 (isključivanje)
        bool cullFaceDisablePressed = (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS);
        if (cullFaceDisablePressed && !cullFaceDisableWasPressed)
        {
            glDisable(GL_CULL_FACE);
        }
        cullFaceDisableWasPressed = cullFaceDisablePressed;

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
        
        // POZIV LIFTA (C) - iz 2D projekta
        // Proveri da li je čovek blizu lifta (ispred lifta, blizu pozicije lifta po Z osi)
        // Lift je na poziciji Z = elevator.z, širina = elevator.width, dubina = elevator.depth
        float elevatorFrontZ = elevator.z - elevator.depth/2.0f;  // Prednja strana lifta (gleda ka -Z)
        // Povećana granica za pozivanje lifta - omogućava pozivanje sa veće udaljenosti (do 1.5m ispred lifta)
        bool nearElevatorFront = (cameraPos.z >= elevatorFrontZ - 1.5f && cameraPos.z <= elevatorFrontZ + 0.3f);
        bool nearElevatorX = (cameraPos.x >= -elevator.width/2.0f - 0.5f && cameraPos.x <= elevator.width/2.0f + 0.5f);
        bool nearElevator = nearElevatorFront && nearElevatorX;
        
        // Čovek poziva lift kada je BLIZU lifta (ispred lifta) i pritisne C
        if (!personHasEnteredElevator &&
            glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS &&
            nearElevator) {
            // Proveri da li je lift na istom spratu (za sada uvek true jer imamo samo jedan sprat)
            bool elevatorAtPersonFloor = true; // Za sada uvek true
            
            if (elevatorAtPersonFloor) {
                // SCENARIO 1: Lift je već na ovom spratu - samo otvori vrata
                doorOpening = true;
                doorClosing = false;
                doorExtended = false;
            }
        }
        
        // OTVARANJE VRATA IZ LIFTA (O) - iz 2D projekta
        // Čovek može da otvori vrata kada je U LIFTU i pritisne O
        if (personHasEnteredElevator && glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
            // SCENARIO 1: Vrata su POTPUNO OTVORENA i mirna
            if (doorOpen && !doorOpening && !doorClosing) {
                // VAŽNO: Proveri da li JOŠ NIJE produženo
                if (!doorExtended) {
                    // Produži otvaranje za još 5 sekundi (SAMO JEDNOM)
                    doorTimerStart = glfwGetTime();  // RESETUJ timer
                    doorExtended = true;             // OZNAČI da je VEĆ produženo
                }
            }
            // SCENARIO 2: Vrata nisu u procesu (zatvorena su)
            else if (!doorOpening && !doorClosing) {
                // Pokreni otvaranje vrata
                doorOpening = true;
                doorClosing = false;
                doorExtended = false;  // Resetuj - nije još produženo
            }
        }
        
        // Ograniči kretanje unutar granica sprata
        // Ako je čovek VAN lifta, ne može da uđe dok vrata nisu otvorena
        // Ako je čovek U LIFTU, ne može da izađe dok vrata nisu otvorena
        bool doorsFullyOpen = (doorOpenAmount >= 1.0f);
        
        // Granice za lift
        float elevatorMinX = -elevator.width/2.0f;
        float elevatorMaxX = elevator.width/2.0f;
        float elevatorMinZ = elevator.z - elevator.depth/2.0f;
        float elevatorMaxZ = elevator.z + elevator.depth/2.0f;
        
        // Proveri da li je čovek u liftu (sa malim marginom)
        float margin = 0.05f; // Mala margina za detekciju
        bool insideElevatorX = (newCameraPos.x >= elevatorMinX - margin && newCameraPos.x <= elevatorMaxX + margin);
        bool insideElevatorZ = (newCameraPos.z >= elevatorMinZ - margin && newCameraPos.z <= elevatorMaxZ + margin);
        bool insideElevator = insideElevatorX && insideElevatorZ;
        
        // Ako je čovek VAN lifta i pokušava da uđe, ali vrata nisu otvorena - blokiraj
        // Ovo se dešava PRE nego što uđe u lift
        if (!personHasEnteredElevator && !doorsFullyOpen) {
            // Ne dozvoli da uđe u lift dok vrata nisu otvorena
            // Blokiraj kretanje ako je blizu lifta (prednja strana)
            // Povećana granica za blokiranje da se izbegne treperenje
            float elevatorFrontZ = elevator.z - elevator.depth/2.0f;  // Prednja strana lifta
            float blockingDistance = 0.5f; // Povećana granica za blokiranje (0.5m ispred lifta)
            bool nearElevatorFront = (newCameraPos.z >= elevatorFrontZ - blockingDistance && newCameraPos.z <= elevatorFrontZ + 0.15f);
            bool nearElevatorX = (newCameraPos.x >= elevatorMinX - 0.15f && newCameraPos.x <= elevatorMaxX + 0.15f);
            
            if (nearElevatorFront && nearElevatorX) {
                // Blokiraj kretanje ka liftu (ne dozvoli da prođe kroz zatvorena vrata)
                // Zadrži ga na sigurnoj udaljenosti ispred lifta
                if (newCameraPos.z > elevatorFrontZ - 0.15f) {
                    newCameraPos.z = elevatorFrontZ - 0.15f; // Zadrži ga ispred lifta (0.15m ispred)
                }
            }
            
            // Takođe, ako je već ušao u lift (unutar granica), vrati ga nazad
            if (insideElevator) {
                // Vrati ga nazad van lifta (ispred prednje strane)
                newCameraPos.z = elevatorFrontZ - 0.2f; // Malo dalje da se izbegne treperenje
                newCameraPos.x = glm::clamp(newCameraPos.x, elevatorMinX - 0.1f, elevatorMaxX + 0.1f);
            }
        }
        
        // Ako je čovek U LIFTU i pokušava da izađe, ali vrata nisu otvorena - blokiraj
        // Povećaj granice blokiranja da se izbegne "treperenje" i vidljivost sprata
        if (personHasEnteredElevator && !doorsFullyOpen) {
            // Strože granice - veći margin da se izbegne izlazak iz opsega
            float strictMargin = 0.3f; // Još veći margin za blokiranje (0.3m sa svake strane)
            float strictMinX = elevatorMinX + strictMargin;
            float strictMaxX = elevatorMaxX - strictMargin;
            float strictMinZ = elevatorMinZ + strictMargin;
            float strictMaxZ = elevatorMaxZ - strictMargin;
            
            // Ne dozvoli da izađe iz lifta dok vrata nisu otvorena
            // Koristi strože granice da se izbegne vidljivost sprata kroz vrata
            newCameraPos.x = glm::clamp(newCameraPos.x, strictMinX, strictMaxX);
            newCameraPos.z = glm::clamp(newCameraPos.z, strictMinZ, strictMaxZ);
        }
        
        // ULAZAK ČOVEKA U LIFT - iz 2D projekta
        bool elevatorAtPersonFloor = true; // Za sada uvek true jer imamo samo jedan sprat
        
        // Čovek automatski ulazi u lift kada:
        // 1. Nije već u liftu
        // 2. Lift je na istom spratu
        // 3. Vrata su potpuno otvorena
        // 4. Čovek je unutar granica lifta (sa marginom)
        if (!personHasEnteredElevator &&
            elevatorAtPersonFloor &&
            doorsFullyOpen &&
            insideElevator) {
            personHasEnteredElevator = true;
            // Pomeri čoveka malo unutar lifta (centar lifta)
            newCameraPos.x = glm::clamp(newCameraPos.x, elevatorMinX + 0.1f, elevatorMaxX - 0.1f);
            newCameraPos.z = glm::clamp(newCameraPos.z, elevatorMinZ + 0.1f, elevatorMaxZ - 0.1f);
        }
        
        // IZLAZAK ČOVEKA IZ LIFTA - iz 2D projekta
        if (personHasEnteredElevator) {
            // Može da izađe kada su vrata otvorena i kada se udalji od lifta
            if (doorsFullyOpen && !insideElevator) {
                personHasEnteredElevator = false;
            }
        }
        
        // Ograniči kretanje unutar granica sprata (ali poštuj ograničenja za lift)
        newCameraPos.x = glm::clamp(newCameraPos.x, minX, maxX);
        newCameraPos.z = glm::clamp(newCameraPos.z, minZ, maxZ);
        newCameraPos.y = cameraY; // Fiksna visina
        cameraPos = newCameraPos;
        
        // ANIMACIJA VRATA - iz 2D projekta
        if (doorOpening) {
            doorOpenAmount += 0.01f;
            // Ako su vrata u potpunosti otvorena
            if (doorOpenAmount >= 1.0f) {
                doorOpenAmount = 1.0f;
                doorOpening = false;
                doorOpen = true;
                doorTimerStart = glfwGetTime();  // POČNI merenje vremena za automatsko zatvaranje
            }
        }
        
        if (doorClosing) {
            doorOpenAmount -= 0.01f;
            // Ako su vrata u potpunosti zatvorena
            if (doorOpenAmount <= 0.0f) {
                doorOpenAmount = 0.0f;
                doorClosing = false;
                doorOpen = false;
            }
        }
        
        // AUTOMATSKO ZATVARANJE nakon 5 sekundi
        if (doorOpen && !doorOpening && !doorClosing) {
            double elapsed = glfwGetTime() - doorTimerStart;
            if (elapsed >= 5.0) {
                doorClosing = true;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(unifiedShader);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
        
        // Postavi pozicije svetlosti (izvori svetlosti u lampama)
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos)); // Pozicija kamere
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(floorLightPos)); // Pozicija lampe na spratu
        glUniform3fv(light2PosLoc, 1, glm::value_ptr(elevatorLightPos)); // Pozicija lampe u liftu
        
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
        elevator.draw(unifiedShader, doorOpenAmount);
        
        // Renderuj lampu za lift sa model shader-om (koristi normalu)
        glUseProgram(modelShader);
        glUniformMatrix4fv(glGetUniformLocation(modelShader, "uM"), 1, GL_FALSE, glm::value_ptr(lampModel));
        glUniformMatrix4fv(glGetUniformLocation(modelShader, "uV"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(modelShader, "uP"), 1, GL_FALSE, glm::value_ptr(projectionP));
        glUniform3f(glGetUniformLocation(modelShader, "uLightPos"), 0.0f, 2.0f, 0.0f);
        glUniform3fv(glGetUniformLocation(modelShader, "uViewPos"), 1, glm::value_ptr(cameraPos));
        glUniform3f(glGetUniformLocation(modelShader, "uLightColor"), 1.0f, 1.0f, 1.0f);
        // Postavi boju za lampu (svetlo žuta/bele boje za lampu)
        glUniform1i(glGetUniformLocation(modelShader, "uUseColor"), 1); // Koristi boju
        glUniform3f(glGetUniformLocation(modelShader, "uModelColor"), 0.95f, 0.95f, 0.85f); // Svetlo žuta/bele boje
        elevatorLamp.Draw(modelShader);
        
        // Renderuj lampu za plafon sprata sa model shader-om (koristi normalu)
        glUniformMatrix4fv(glGetUniformLocation(modelShader, "uM"), 1, GL_FALSE, glm::value_ptr(floorLampModel));
        floorLamp.Draw(modelShader);
        
        glUniform1i(glGetUniformLocation(modelShader, "uUseColor"), 0); // Vrati nazad za biljku
        
        // Renderuj biljku sa model shader-om (koristi normalu)
        glUniformMatrix4fv(glGetUniformLocation(modelShader, "uM"), 1, GL_FALSE, glm::value_ptr(plantModel));
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
