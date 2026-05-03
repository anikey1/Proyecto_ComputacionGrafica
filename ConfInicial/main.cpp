// Proyecto Final - IXANIK
// Integrantes:
// 320260366
// 319323290
// 320110450

#include <string>
#include <iostream>
#include <vector>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Configurar posición inicial de la cámara (altura y posición en el pasillo)
Camera camera(glm::vec3(18.0f, 2.2f, 31.0f));
bool keys[1024];
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

struct Stand {
    std::string path;
    glm::vec3 pos;
    glm::vec3 scale;
    float rotY;
    bool visible;
};

std::vector<Stand> standConfigs = {
    { "Models/Stands/s1/principal.obj", glm::vec3(8.0f, 0.0f,  25.5f), glm::vec3(1.05f), 0.0f,  true },
    { "Models/Stands/s2/s2.obj",        glm::vec3(-9.0f, 0.0f,  4.4f), glm::vec3(0.9f),  90.0f, true },
    { "Models/Stands/s3/s3.obj",        glm::vec3(5.7f, 0.0f,   9.2f), glm::vec3(0.9f),  90.0f, true },
    { "Models/Stands/s4/s4.obj",        glm::vec3(5.7f, 0.0f,  14.0f), glm::vec3(0.01f), 90.0f, true },
    { "Models/Stands/s5/s5.obj",        glm::vec3(5.9f, 0.0f,  -1.0f), glm::vec3(0.9f),  90.0f, true },
    { "Models/Stands/s6/s6.obj",        glm::vec3(5.7f, 0.0f,  -7.0f), glm::vec3(0.85f), 0.0f,  true },
    { "Models/Stands/s7/s7.obj",        glm::vec3(5.7f, 0.0f, -11.0f), glm::vec3(0.8f),  0.0f,  true },
    { "Models/Stands/s8/s8.obj",        glm::vec3(5.7f, 0.0f, -16.0f), glm::vec3(1.0f),  0.0f,  true },
};

std::vector<Model*> stands;
// Inicializar sin ningún stand seleccionado por defecto
int selectedStand = -1;

// ============================================================ 
// PÁJARO - ANIMACIÓN
// ============================================================ 
Model* birdBody = nullptr;
Model* birdHead = nullptr;
Model* birdWingR = nullptr;
Model* birdWingRT = nullptr;
Model* birdWingL = nullptr;
Model* birdWingLT = nullptr;
Model* birdTail = nullptr;
float birdDirZ = 1.0f;

float birdFacingAngle = 270.0f;
float birdTargetAngle = 270.0f;
bool  birdTurning = false;
float birdBankAngle = 0.0f;

// Definir la posición inicial y final del pájaro en el lobby
glm::vec3 birdPos = glm::vec3(9.0f, 3.9f, 25.0f);
const glm::vec3 BIRD_START = glm::vec3(9.0f, 3.9f, 25.0f);
const glm::vec3 BIRD_END = glm::vec3(9.0f, 3.9f, -30.0f);
const float BIRD_SPEED = 3.0f;
bool birdVisible = true;

float wingRightAngle = 0.0f;
float wingLeftAngle = 0.0f;

#define BIRD_MAX_FRAMES 3
typedef struct {
    float wingAngle;
    float wingAngleInc;
} BirdWingFrame;

BirdWingFrame birdFrames[BIRD_MAX_FRAMES];
int birdMaxSteps = 70;
int birdCurrSteps = 0;
int birdPlayIndex = 0;

void BirdWingInterpolation() {
    int current = birdPlayIndex;
    int next = (birdPlayIndex + 1) % BIRD_MAX_FRAMES;
    birdFrames[current].wingAngleInc =
        (birdFrames[next].wingAngle - birdFrames[current].wingAngle)
        / (float)birdMaxSteps;
}

void AnimateBirdWings() {
    if (birdCurrSteps >= birdMaxSteps) {
        birdCurrSteps = 0;
        birdPlayIndex = (birdPlayIndex + 1) % BIRD_MAX_FRAMES;
        BirdWingInterpolation();
    }
    wingRightAngle += birdFrames[birdPlayIndex].wingAngleInc;
    wingLeftAngle = wingRightAngle;
    birdCurrSteps++;
}

void UpdateBird() {
    if (birdTurning) {
        float diff = birdTargetAngle - birdFacingAngle;
        birdFacingAngle += diff * 1.2f * deltaTime;
        birdBankAngle = diff * 0.6f;
        if (fabs(diff) < 1.0f) {
            birdFacingAngle = birdTargetAngle;
            birdBankAngle = 0.0f;
            birdTurning = false;
        }
        birdPos.z += birdDirZ * BIRD_SPEED * 0.3f * deltaTime;
        birdPos.y = BIRD_START.y;
        AnimateBirdWings();
        return;
    }

    birdPos.z += birdDirZ * BIRD_SPEED * deltaTime;
    birdPos.y = BIRD_START.y + 0.3f * (float)sin(glfwGetTime() * 2.0);
    birdPos.x = BIRD_START.x + 0.4f * (float)sin(glfwGetTime() * 0.7);

    if (birdPos.z <= BIRD_END.z) {
        birdPos.z = BIRD_END.z + 0.1f;
        birdDirZ = 1.0f;
        birdTargetAngle = birdFacingAngle + 180.0f;
        birdTurning = true;
    }
    if (birdPos.z >= BIRD_START.z) {
        birdPos.z = BIRD_START.z - 0.1f;
        birdDirZ = -1.0f;
        birdTargetAngle = birdFacingAngle - 180.0f;
        birdTurning = true;
    }

    AnimateBirdWings();
}

// ============================================================ 
// ARDILLA - MODELADO JERÁRQUICO Y PATH ANIMATION
// ============================================================ 
Model* sqBody = nullptr;
Model* sqLeg1 = nullptr;
Model* sqLeg2 = nullptr;
Model* sqArm1 = nullptr;
Model* sqArm2 = nullptr;
Model* sqTail = nullptr;

bool sqVisible = true;

// Configurar el sistema de Waypoints para que el modelo esquive stands y paredes
std::vector<glm::vec3> sqPath = {
    glm::vec3(15.0f, 0.5f,  25.0f), // 0: Iniciar en la zona del pasillo
    glm::vec3(9.5f,  0.5f,  20.0f), // 1: Ajustar al centro del pasillo
    glm::vec3(9.5f,  0.5f,   0.0f), // 2: Avanzar por la zona central libre
    glm::vec3(9.5f,  0.5f, -15.0f), // 3: Continuar en línea recta
    glm::vec3(5.0f,  0.5f, -20.0f)  // 4: Finalizar el recorrido en el poste
};

int currentWP = 0;
float sqLerpT = 0.0f;
glm::vec3 sqPos = sqPath[0];
float sqYaw = 0.0f;
float sqWalkAngle = 0.0f;

// Calcular las actualizaciones matemáticas de posición y rotación de la ardilla
void UpdateSquirrel() {
    float sqSpeed = 0.5f;
    sqLerpT += sqSpeed * deltaTime;

    int nextWP = currentWP + 1;

    // Avanzar al siguiente waypoint al completar un segmento
    if (sqLerpT > 1.0f) {
        sqLerpT = 0.0f;
        currentWP++;
        nextWP++;

        // Reiniciar el recorrido al llegar al final de la lista de puntos
        if (currentWP >= sqPath.size() - 1) {
            currentWP = 0;
            nextWP = 1;
        }
    }

    // Almacenar los puntos de inicio y fin actuales para la interpolación
    glm::vec3 pStart = sqPath[currentWP];
    glm::vec3 pEnd = sqPath[nextWP];

    // Aplicar Interpolación Lineal (LERP) para determinar la posición exacta
    sqPos = pStart + sqLerpT * (pEnd - pStart);

    // Calcular la rotación direccional para mirar hacia el siguiente waypoint
    glm::vec3 sqDir = glm::normalize(pEnd - pStart);
    sqYaw = glm::degrees(atan2(sqDir.x, sqDir.z));

    // Agregar oscilación en el eje Y (con valor absoluto) para simular movimiento de salto
    sqPos.y = 0.5f + std::abs((float)sin(glfwGetTime() * 15.0)) * 0.3f;

    // Animación de ciclo: Aplicar función seno para el movimiento continuo de las extremidades
    sqWalkAngle = (float)(sin(glfwGetTime() * 15.0) * 25.0);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "IXANIK - Lobby FI", nullptr, nullptr);
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Activar Depth Test para manejar la oclusión espacial de modelos 3D
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // Instanciar todos los modelos de los stands del arreglo
    for (int i = 0; i < (int)standConfigs.size(); i++)
        stands.push_back(new Model((char*)standConfigs[i].path.c_str()));

    // Cargar los archivos del modelo jerárquico del pájaro
    birdBody = new Model((char*)"Models/Bird/cuerpo.obj");
    birdHead = new Model((char*)"Models/Bird/cabeza.obj");
    birdWingR = new Model((char*)"Models/Bird/alaDer.obj");
    birdWingRT = new Model((char*)"Models/Bird/alaDer_Punta.obj");
    birdWingL = new Model((char*)"Models/Bird/alaIzq.obj");
    birdWingLT = new Model((char*)"Models/Bird/alaIzq_punt.obj");
    birdTail = new Model((char*)"Models/Bird/cola.obj");

    // Cargar los archivos del modelo jerárquico de la ardilla
    sqBody = new Model((char*)"Models/ardilla/cuerpo.obj");
    sqLeg1 = new Model((char*)"Models/ardilla/pata1.obj");
    sqLeg2 = new Model((char*)"Models/ardilla/pata2.obj");
    sqArm1 = new Model((char*)"Models/ardilla/mano1.obj");
    sqArm2 = new Model((char*)"Models/ardilla/mano2.obj");
    sqTail = new Model((char*)"Models/ardilla/cola.obj");

    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.01f, 1000.0f
    );

    // Cargar la geometría principal del escenario (Lobby)
    Model lobby((char*)"Models/ModeloLobby/final.obj");

    birdFrames[0].wingAngle = 35.0f;
    birdFrames[1].wingAngle = 0.0f;
    birdFrames[2].wingAngle = -20.0f;
    BirdWingInterpolation();

    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        // Procesar la lógica de transformaciones antes de realizar el renderizado
        UpdateBird();
        UpdateSquirrel();

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // Definir parámetros y ubicación de la luz para el fragment shader
        glm::vec3 camPos = camera.GetPosition();
        glUniform3f(glGetUniformLocation(shader.Program, "viewPos"), camPos.x, camPos.y, camPos.z);
        glUniform3f(glGetUniformLocation(shader.Program, "lightDir"), -0.3f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "lightAmbient"), 0.55f, 0.55f, 0.55f);
        glUniform3f(glGetUniformLocation(shader.Program, "lightDiffuse"), 0.6f, 0.6f, 0.6f);
        glUniform3f(glGetUniformLocation(shader.Program, "lightSpecular"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(shader.Program, "fillLightDir"), 0.3f, 0.5f, 0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "fillLightColor"), 0.25f, 0.25f, 0.28f);

        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Dibujar entorno estático del Lobby
        glm::mat4 modelLobby = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelLobby));
        lobby.Draw(shader);

        // Iterar sobre el vector para dibujar y posicionar los stands
        for (int i = 0; i < (int)stands.size(); i++) {
            if (!standConfigs[i].visible) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, standConfigs[i].pos);
            model = glm::rotate(model, glm::radians(standConfigs[i].rotY), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, standConfigs[i].scale);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            stands[i]->Draw(shader);
        }


        // ============================================================
        // RENDERIZAR PÁJARO
        // ============================================================
        if (birdVisible) {
            glm::mat4 birdBase = glm::mat4(1.0f);
            birdBase = glm::translate(birdBase, birdPos);
            birdBase = glm::rotate(birdBase, glm::radians(birdFacingAngle), glm::vec3(0, 1, 0));
            birdBase = glm::rotate(birdBase, glm::radians(birdBankAngle), glm::vec3(0, 0, 1));
            birdBase = glm::rotate(birdBase, glm::radians(-45.0f), glm::vec3(0, 0, 1));
            birdBase = glm::scale(birdBase, glm::vec3(0.25f));

            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(birdBase));
            birdBody->Draw(shader);

            // Cabeza
            glm::vec3 pivotHead = glm::vec3(1.651f, -0.286f, -0.345f);
            float headYaw = (float)(sin(glfwGetTime() * 0.8) * 6.0);
            float headPitch = -wingRightAngle * 0.15f;
            glm::mat4 headMat = birdBase;
            headMat = glm::translate(headMat, pivotHead);
            headMat = glm::rotate(headMat, glm::radians(headYaw), glm::vec3(0, 1, 0));
            headMat = glm::rotate(headMat, glm::radians(headPitch), glm::vec3(0, 0, 1));
            headMat = glm::translate(headMat, -pivotHead);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(headMat));
            birdHead->Draw(shader);

            // Cola
            glm::vec3 pivotTail = glm::vec3(0.389f, -1.266f, -0.310f);
            float tailPitch = wingRightAngle * 0.2f;
            glm::mat4 tailMat = birdBase;
            tailMat = glm::translate(tailMat, pivotTail);
            tailMat = glm::rotate(tailMat, glm::radians(tailPitch), glm::vec3(0, 0, 1));
            tailMat = glm::translate(tailMat, -pivotTail);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(tailMat));
            birdTail->Draw(shader);

            // Ala Derecha
            glm::vec3 pivotR = glm::vec3(1.05f, -0.66f, 0.0f);
            glm::mat4 wingRRoot = birdBase;
            wingRRoot = glm::translate(wingRRoot, pivotR);
            wingRRoot = glm::rotate(wingRRoot, glm::radians(wingRightAngle), glm::vec3(1, 0, 0));
            wingRRoot = glm::translate(wingRRoot, -pivotR);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingRRoot));
            birdWingR->Draw(shader);

            // Punta del Ala Derecha
            glm::vec3 pivotRT = glm::vec3(1.05f, -0.66f, 0.44f);
            float tipFoldR = wingRightAngle * 0.7f - 10.0f;
            glm::mat4 wingRTip = wingRRoot;
            wingRTip = glm::translate(wingRTip, pivotRT);
            wingRTip = glm::rotate(wingRTip, glm::radians(tipFoldR), glm::vec3(1, 0, 0));
            wingRTip = glm::translate(wingRTip, -pivotRT);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingRTip));
            birdWingRT->Draw(shader);

            // Ala Izquierda
            glm::vec3 pivotL = glm::vec3(1.15f, -0.66f, -0.33f);
            glm::mat4 wingLRoot = birdBase;
            wingLRoot = glm::translate(wingLRoot, pivotL);
            wingLRoot = glm::rotate(wingLRoot, glm::radians(-wingRightAngle), glm::vec3(1, 0, 0));
            wingLRoot = glm::translate(wingLRoot, -pivotL);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingLRoot));
            birdWingL->Draw(shader);

            // Punta del Ala Izquierda
            glm::vec3 pivotLT = glm::vec3(1.15f, -0.66f, -0.65f);
            float tipFoldL = -wingRightAngle * 0.7f + 10.0f;
            glm::mat4 wingLTip = wingLRoot;
            wingLTip = glm::translate(wingLTip, pivotLT);
            wingLTip = glm::rotate(wingLTip, glm::radians(tipFoldL), glm::vec3(1, 0, 0));
            wingLTip = glm::translate(wingLTip, -pivotLT);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingLTip));
            birdWingLT->Draw(shader);
        }

        // ============================================================
        // RENDERIZAR ARDILLA
        // ============================================================
        if (sqVisible) {
            glm::mat4 sqBase = glm::mat4(1.0f);

            // Trasladar a la posición calculada mediante LERP
            sqBase = glm::translate(sqBase, sqPos);

            // Aplicar la rotación para alinear el modelo con la trayectoria
            sqBase = glm::rotate(sqBase, glm::radians(sqYaw), glm::vec3(0, 1, 0));

            // Reducir la escala (0.3) para evitar colisiones volumétricas en el escenario
            sqBase = glm::scale(sqBase, glm::vec3(0.3f));

            // Dibujar el torso como centro del modelado jerárquico
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(sqBase));
            sqBody->Draw(shader);

            // Cola 
            // Invertir Y y Z en los pivotes generados desde Blender para empatar con OpenGL
            glm::vec3 pivotCola = glm::vec3(0.2569f, -0.12934f, -0.21342f);
            glm::mat4 colaMat = sqBase;
            colaMat = glm::translate(colaMat, pivotCola);
            float anguloCola = (float)(sin(glfwGetTime() * 5.0) * 15.0);
            colaMat = glm::rotate(colaMat, glm::radians(anguloCola), glm::vec3(1, 0, 0));
            colaMat = glm::translate(colaMat, -pivotCola);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(colaMat));
            sqTail->Draw(shader);

            // Mano 1 (Delantera Izquierda)
            glm::vec3 pivotMano1 = glm::vec3(0.2569f, -0.12934f, -0.21342f);
            glm::mat4 mano1Mat = sqBase;
            mano1Mat = glm::translate(mano1Mat, pivotMano1);
            mano1Mat = glm::rotate(mano1Mat, glm::radians(sqWalkAngle), glm::vec3(1, 0, 0));
            mano1Mat = glm::translate(mano1Mat, -pivotMano1);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mano1Mat));
            sqArm1->Draw(shader);

            // Mano 2 (Delantera Derecha)
            glm::vec3 pivotMano2 = glm::vec3(0.2569f, -0.12934f, 0.21342f);
            glm::mat4 mano2Mat = sqBase;
            mano2Mat = glm::translate(mano2Mat, pivotMano2);
            // Rotación invertida para simular el desplazamiento natural alternado
            mano2Mat = glm::rotate(mano2Mat, glm::radians(-sqWalkAngle), glm::vec3(1, 0, 0));
            mano2Mat = glm::translate(mano2Mat, -pivotMano2);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mano2Mat));
            sqArm2->Draw(shader);

            // Pata 1 (Trasera Izquierda)
            glm::vec3 pivotPata1 = glm::vec3(-0.34106f, -1.1076f, -0.61203f);
            glm::mat4 pata1Mat = sqBase;
            pata1Mat = glm::translate(pata1Mat, pivotPata1);
            pata1Mat = glm::rotate(pata1Mat, glm::radians(-sqWalkAngle), glm::vec3(1, 0, 0));
            pata1Mat = glm::translate(pata1Mat, -pivotPata1);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(pata1Mat));
            sqLeg1->Draw(shader);

            // Pata 2 (Trasera Derecha)
            glm::vec3 pivotPata2 = glm::vec3(0.70116f, -1.109f, -0.65543f);
            glm::mat4 pata2Mat = sqBase;
            pata2Mat = glm::translate(pata2Mat, pivotPata2);
            pata2Mat = glm::rotate(pata2Mat, glm::radians(sqWalkAngle), glm::vec3(1, 0, 0));
            pata2Mat = glm::translate(pata2Mat, -pivotPata2);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(pata2Mat));
            sqLeg2->Draw(shader);
        }

        glfwSwapBuffers(window);
    }

    // Liberar la memoria dinámica de los modelos al cerrar la ventana
    for (Model* s : stands) delete s;
    stands.clear();

    delete birdBody;
    delete birdHead;
    delete birdWingR;
    delete birdWingRT;
    delete birdWingL;
    delete birdWingLT;
    delete birdTail;

    delete sqBody;
    delete sqLeg1;
    delete sqLeg2;
    delete sqArm1;
    delete sqArm2;
    delete sqTail;

    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);

    // Controles de transformación para modificar los stands seleccionados
    if (selectedStand >= 0) {
        Stand& s = standConfigs[selectedStand];
        float moveSpeed = 5.0f * deltaTime;
        float rotSpeed = 90.0f * deltaTime;
        float scaleSpeed = 1.0f * deltaTime;

        // Desplazamiento posicional con flechas direccionales y teclas de página
        if (keys[GLFW_KEY_RIGHT])     s.pos.x += moveSpeed;
        if (keys[GLFW_KEY_LEFT])      s.pos.x -= moveSpeed;
        if (keys[GLFW_KEY_UP])        s.pos.z -= moveSpeed;
        if (keys[GLFW_KEY_DOWN])      s.pos.z += moveSpeed;
        if (keys[GLFW_KEY_PAGE_UP])   s.pos.y += moveSpeed;
        if (keys[GLFW_KEY_PAGE_DOWN]) s.pos.y -= moveSpeed;

        // Modificación de ángulo de rotación Y
        if (keys[GLFW_KEY_Q]) s.rotY -= rotSpeed;
        if (keys[GLFW_KEY_E]) s.rotY += rotSpeed;

        // Modificación de dimensiones mediante escalado global
        if (keys[GLFW_KEY_Z]) s.scale -= glm::vec3(scaleSpeed);
        if (keys[GLFW_KEY_X]) s.scale += glm::vec3(scaleSpeed);
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)        keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    // Asignar el índice del stand seleccionado en base al número introducido
    if (key == GLFW_KEY_1) selectedStand = 0;
    if (key == GLFW_KEY_2) selectedStand = 1;
    if (key == GLFW_KEY_3) selectedStand = 2;
    if (key == GLFW_KEY_4) selectedStand = 3;
    if (key == GLFW_KEY_5) selectedStand = 4;
    if (key == GLFW_KEY_6) selectedStand = 5;
    if (key == GLFW_KEY_7) selectedStand = 6;
    if (key == GLFW_KEY_8) selectedStand = 7;

    // Deseleccionar objetos activos
    if (key == GLFW_KEY_0) selectedStand = -1;

    // Conmutar el estado de visibilidad del stand actual
    if (key == GLFW_KEY_H && selectedStand >= 0)
        standConfigs[selectedStand].visible = !standConfigs[selectedStand].visible;
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) { lastX = xPos; lastY = yPos; firstMouse = false; }
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    lastX = xPos; lastY = yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}