//Proyecto Final - IXANIK
// Integrantes:
//319323290
//320260366

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
///                     .., altura, largo pasiillo
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
/// extra de stans 
int selectedStand = -1;  // ninguno seleccionado

//  PÁJARO - KEYFRAME ANIMATION
Model* birdBody = nullptr;
Model* birdHead = nullptr;
Model* birdWingR = nullptr;
Model* birdWingRT = nullptr;
Model* birdWingL = nullptr;
Model* birdWingLT = nullptr;
Model* birdTail = nullptr;
float birdDirZ = 1.0f;

float birdFacingAngle = 270.0f;  // ángulo actual de rotación Y
float birdTargetAngle = 270.0f;  // ángulo destino
bool  birdTurning = false;
float birdBankAngle = 0.0f;    // inclinación durante el giro

glm::vec3 birdPos = glm::vec3(9.0f, 3.9f,25.0f);
const glm::vec3 BIRD_START = glm::vec3(9.0f, 3.9f, 25.0f);
const glm::vec3 BIRD_END = glm::vec3(9.0f   , 3.9f, -30.0f);
const float     BIRD_SPEED = 3.0f;//subir mas rapido lit
bool            birdVisible = true;

float birdPauseTimer = 0.0f;
const float BIRD_PAUSE = 1.5f;
bool  birdPausing = false;

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
        birdPos.y = BIRD_START.y;  // altura fija durante el giro, sin sin()
        AnimateBirdWings();
        return;
    }

    birdPos.z += birdDirZ * BIRD_SPEED * deltaTime;
    birdPos.y = BIRD_START.y + 0.3f * sin(glfwGetTime() * 2.0f); // ondeo solo fuera del giro

    birdPos.x = BIRD_START.x + 0.4f * sin(glfwGetTime() * 0.7f);

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
// ============================================================ PAJARO


//  PERSONA - KEYFRAME ANIMATION CON CAMINATA LENTA

Model* personBody = nullptr;
Model* personRightArm = nullptr;
Model* personLeftArm = nullptr;
Model* personRightLeg = nullptr;
Model* personLeftLeg = nullptr;
bool  personVisible = true;

// Posición fija de la persona
const glm::vec3 PERSON_STAND_POS = glm::vec3(11.0f, 0.0f, 25.0f);
// Punto hacia donde mira/señala, aproximadamente el stand
const glm::vec3 PERSON_LOOK_STAND = glm::vec3(8.0f, 0.0f, 25.5f);
// Ajuste general de orientacion del modelo.
glm::vec3 personScale = glm::vec3(1.0f);
const float PERSON_MODEL_FORWARD_OFFSET = 160.0f;


// Valores que se actualizan cada frame.
glm::vec3 personPos = PERSON_STAND_POS; 
float personYaw = 0.0f;
float personBodySideLean = 0.0f;
float personBodyForwardLean = 0.0f;
float personBodyBob = 0.0f;
float personRightArmZ = -75.0f;
float personLeftArmZ = 75.0f;
float personRightArmX = 0.0f;
float personLeftArmX = 0.0f;
float personRightLegX = 0.0f;
float personLeftLegX = 0.0f;

float personAnimTime = 0.0f;

float Clamp01(float t) {
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}

float SmoothStep(float t) {
    t = Clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

float LerpFloat(float a, float b, float t) {
    return a + (b - a) * t;
}

glm::vec3 LerpVec3(glm::vec3 a, glm::vec3 b, float t) {
    return a + (b - a) * t;
}

float NormalizarAngulo(float angulo) {
    while (angulo > 180.0f) angulo -= 360.0f;
    while (angulo < -180.0f) angulo += 360.0f;
    return angulo;
}

float LerpAngulo(float actual, float objetivo, float t) {
    float diferencia = NormalizarAngulo(objetivo - actual);
    return actual + diferencia * t;
}

float YawHaciaPunto(glm::vec3 desde, glm::vec3 hacia) {
    glm::vec3 dir = hacia - desde;

    return glm::degrees(atan2(dir.x, dir.z)) + PERSON_MODEL_FORWARD_OFFSET;
}

void ResetPoseReposo() {
    personBodyBob = 0.0f;
    personBodySideLean = 0.0f;
    personBodyForwardLean = 0.0f;
    personRightArmZ = -75.0f;
    personLeftArmZ = 75.0f;
    personRightArmX = 0.0f;
    personLeftArmX = 0.0f;
    personRightLegX = 0.0f;
    personLeftLegX = 0.0f;
}

void UpdatePersonWalkStand() {
    personAnimTime += deltaTime;

    // La persona siempre se queda fija en esta posición
    personPos = PERSON_STAND_POS;

    // La persona mira hacia el stand
    float yawLookStand = YawHaciaPunto(PERSON_STAND_POS, PERSON_LOOK_STAND);

    // Ajuste fino de orientación.
    // Si no mira bien al stand, cambia este valor.
    yawLookStand += 8.0f;

    personYaw = yawLookStand;

    ResetPoseReposo();

    // ---------- TIEMPOS DEL GESTO ----------
    const float REST_TIME = 1.0f;       // espera en reposo
    const float ARM_UP_TIME = 1.4f;     // levanta la mano
    const float HOLD_TIME = 2.5f;       // mantiene señalando
    const float ARM_DOWN_TIME = 1.3f;   // baja la mano
    const float PAUSE_TIME = 1.2f;      // pausa antes de repetir

    const float CYCLE_TIME = REST_TIME + ARM_UP_TIME + HOLD_TIME + ARM_DOWN_TIME + PAUSE_TIME;

    float t = fmod(personAnimTime, CYCLE_TIME);

    // ---------- ÁNGULOS DEL BRAZO ----------
    // X mueve el brazo hacia enfrente
    // Z controla qué tan abierto queda hacia un lado
    const float ARM_FORWARD_X = 70.0f;
    const float ARM_FORWARD_Z = -65.0f;

    // 1) Reposo
    if (t < REST_TIME) {
        return;
    }

    // 2) Levanta la mano
    else if (t < REST_TIME + ARM_UP_TIME) {
        float p = (t - REST_TIME) / ARM_UP_TIME;
        p = SmoothStep(p);

        personRightArmX = LerpFloat(0.0f, ARM_FORWARD_X, p);
        personRightArmZ = LerpFloat(-75.0f, ARM_FORWARD_Z, p);

        personBodyForwardLean = LerpFloat(0.0f, -2.0f, p);
    }

    // 3) Mantiene la mano señalando
    else if (t < REST_TIME + ARM_UP_TIME + HOLD_TIME) {
        personRightArmX = ARM_FORWARD_X + sin(glfwGetTime() * 1.4f) * 1.5f;
        personRightArmZ = ARM_FORWARD_Z;

        personBodyForwardLean = -2.0f;
    }

    // 4) Baja la mano
    else if (t < REST_TIME + ARM_UP_TIME + HOLD_TIME + ARM_DOWN_TIME) {
        float p = (t - (REST_TIME + ARM_UP_TIME + HOLD_TIME)) / ARM_DOWN_TIME;
        p = SmoothStep(p);

        personRightArmX = LerpFloat(ARM_FORWARD_X, 0.0f, p);
        personRightArmZ = LerpFloat(ARM_FORWARD_Z, -75.0f, p);

        personBodyForwardLean = LerpFloat(-2.0f, 0.0f, p);
    }

    // 5) Pausa final
    else {
        return;
    }
}

//============================================================ PERSONA
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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    for (int i = 0; i < (int)standConfigs.size(); i++)
        stands.push_back(new Model((char*)standConfigs[i].path.c_str()));

    birdBody = new Model((char*)"Models/Bird/cuerpo.obj");
    birdHead = new Model((char*)"Models/Bird/cabeza.obj");
    birdWingR = new Model((char*)"Models/Bird/alaDer.obj");
    birdWingRT = new Model((char*)"Models/Bird/alaDer_Punta.obj");
    birdWingL = new Model((char*)"Models/Bird/alaIzq.obj");
    birdWingLT = new Model((char*)"Models/Bird/alaIzq_punt.obj");
    birdTail = new Model((char*)"Models/Bird/cola.obj");


    // Modelo de la persona separado por partes para poder animar los brazos.
    personBody = new Model((char*)"Models/Persona/persona_cuerpo.obj");
    personRightArm = new Model((char*)"Models/Persona/persona_brazo_derecho.obj");
    personLeftArm = new Model((char*)"Models/Persona/persona_brazo_izquierdo.obj");
    personRightLeg = new Model((char*)"Models/Persona/persona_pierna_derecha.obj");
    personLeftLeg = new Model((char*)"Models/Persona/persona_pierna_izquierda.obj");


    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.01f, 1000.0f
    );

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
        UpdateBird();
        UpdatePersonWalkStand();

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

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

        // Lobby
        glm::mat4 modelLobby = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelLobby));
        lobby.Draw(shader);

        // Stands
        for (int i = 0; i < (int)stands.size(); i++) {
            if (!standConfigs[i].visible) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, standConfigs[i].pos);
            model = glm::rotate(model, glm::radians(standConfigs[i].rotY), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, standConfigs[i].scale);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            stands[i]->Draw(shader);
        }

       
        //  DIBUJAR PÁJARO
        if (birdVisible) {

            glm::mat4 birdBase = glm::mat4(1.0f);
            birdBase = glm::translate(birdBase, birdPos);
            birdBase = glm::rotate(birdBase, glm::radians(birdFacingAngle), glm::vec3(0, 1, 0)); // orientación
            birdBase = glm::rotate(birdBase, glm::radians(birdBankAngle), glm::vec3(0, 0, 1)); // bank/inclinación lateral
            birdBase = glm::rotate(birdBase, glm::radians(-45.0f), glm::vec3(0, 0, 1)); // tu inclinación fija
            birdBase = glm::scale(birdBase, glm::vec3(0.25f));

            // Cuerpo
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(birdBase));
            birdBody->Draw(shader);

            // --- CABEZA ---
            glm::vec3 pivotHead = glm::vec3(1.651f, -0.286f, -0.345f);
            float headYaw = sin(glfwGetTime() * 0.8f) * 6.0f;   // lado a lado, más amplitud
            float headPitch = -wingRightAngle * 0.15f;              // más notorio
            glm::mat4 headMat = birdBase;
            headMat = glm::translate(headMat, pivotHead);
            headMat = glm::rotate(headMat, glm::radians(headYaw), glm::vec3(0, 1, 0));
            headMat = glm::rotate(headMat, glm::radians(headPitch), glm::vec3(0, 0, 1)); // cambia a Z
            headMat = glm::translate(headMat, -pivotHead);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(headMat));
            birdHead->Draw(shader);

            // --- COLA ---
            glm::vec3 pivotTail = glm::vec3(0.389f, -1.266f, -0.310f);
            float tailPitch = wingRightAngle * 0.2f;  // más amplitud
            glm::mat4 tailMat = birdBase;
            tailMat = glm::translate(tailMat, pivotTail);
            tailMat = glm::rotate(tailMat, glm::radians(tailPitch), glm::vec3(0, 0, 1)); // cambia a Z
            tailMat = glm::translate(tailMat, -pivotTail);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(tailMat));
            birdTail->Draw(shader);

            // --- ALA DERECHA raíz ---
            glm::vec3 pivotR = glm::vec3(1.05f, -0.66f, 0.0f);
            glm::mat4 wingRRoot = birdBase;
            wingRRoot = glm::translate(wingRRoot, pivotR);
            wingRRoot = glm::rotate(wingRRoot, glm::radians(wingRightAngle), glm::vec3(1, 0, 0));
            wingRRoot = glm::translate(wingRRoot, -pivotR);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingRRoot));
            birdWingR->Draw(shader);

            // --- ALA DERECHA punta ---
            glm::vec3 pivotRT = glm::vec3(1.05f, -0.66f, 0.44f);
            float tipFoldR = wingRightAngle * 0.7f - 10.0f;
            glm::mat4 wingRTip = wingRRoot;
            wingRTip = glm::translate(wingRTip, pivotRT);
            wingRTip = glm::rotate(wingRTip, glm::radians(tipFoldR), glm::vec3(1, 0, 0));
            wingRTip = glm::translate(wingRTip, -pivotRT);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingRTip));
            birdWingRT->Draw(shader);

            // --- ALA IZQUIERDA raíz ---
            glm::vec3 pivotL = glm::vec3(1.15f, -0.66f, -0.33f);
            glm::mat4 wingLRoot = birdBase;
            wingLRoot = glm::translate(wingLRoot, pivotL);
            wingLRoot = glm::rotate(wingLRoot, glm::radians(-wingRightAngle), glm::vec3(1, 0, 0));
            wingLRoot = glm::translate(wingLRoot, -pivotL);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingLRoot));
            birdWingL->Draw(shader);

            // --- ALA IZQUIERDA punta ---
            glm::vec3 pivotLT = glm::vec3(1.15f, -0.66f, -0.65f);
            float tipFoldL = -wingRightAngle * 0.7f + 10.0f;
            glm::mat4 wingLTip = wingLRoot;
            wingLTip = glm::translate(wingLTip, pivotLT);
            wingLTip = glm::rotate(wingLTip, glm::radians(tipFoldL), glm::vec3(1, 0, 0));
            wingLTip = glm::translate(wingLTip, -pivotLT);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingLTip));
            birdWingLT->Draw(shader);
        }


        //  DIBUJAR PERSONA CAMINANDO E INTERACTUANDO CON EL STAND
        if (personVisible && personBody != nullptr && personRightArm != nullptr && personLeftArm != nullptr
            && personRightLeg != nullptr && personLeftLeg != nullptr) {

            glm::mat4 personBase = glm::mat4(1.0f);
            personBase = glm::translate(personBase, personPos + glm::vec3(0.0f, personBodyBob, 0.0f));
            personBase = glm::rotate(personBase, glm::radians(personYaw), glm::vec3(0.0f, 1.0f, 0.0f));
            personBase = glm::rotate(personBase, glm::radians(personBodyForwardLean), glm::vec3(1.0f, 0.0f, 0.0f));
            personBase = glm::rotate(personBase, glm::radians(personBodySideLean), glm::vec3(0.0f, 0.0f, 1.0f));
            personBase = glm::scale(personBase, personScale);

            // Cuerpo sin brazos ni piernas
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(personBase));
            personBody->Draw(shader);

            // Brazo derecho
            glm::vec3 pivotRightArm = glm::vec3(0.10f, 1.52f, 0.08f);
            glm::mat4 rightArmMat = personBase;
            rightArmMat = glm::translate(rightArmMat, pivotRightArm);
            rightArmMat = glm::rotate(rightArmMat, glm::radians(personRightArmX), glm::vec3(1.0f, 0.0f, 0.0f));
            rightArmMat = glm::rotate(rightArmMat, glm::radians(personRightArmZ), glm::vec3(0.0f, 0.0f, 1.0f));
            rightArmMat = glm::translate(rightArmMat, -pivotRightArm);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(rightArmMat));
            personRightArm->Draw(shader);

            // Brazo izquierdo
            glm::vec3 pivotLeftArm = glm::vec3(-0.10f, 1.52f, 0.08f);
            glm::mat4 leftArmMat = personBase;
            leftArmMat = glm::translate(leftArmMat, pivotLeftArm);
            leftArmMat = glm::rotate(leftArmMat, glm::radians(personLeftArmX), glm::vec3(1.0f, 0.0f, 0.0f));
            leftArmMat = glm::rotate(leftArmMat, glm::radians(personLeftArmZ), glm::vec3(0.0f, 0.0f, 1.0f));
            leftArmMat = glm::translate(leftArmMat, -pivotLeftArm);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(leftArmMat));
            personLeftArm->Draw(shader);

            // Pierna derecha
            glm::vec3 pivotRightLeg = glm::vec3(0.10f, 1.02f, 0.03f);
            glm::mat4 rightLegMat = personBase;
            rightLegMat = glm::translate(rightLegMat, pivotRightLeg);
            rightLegMat = glm::rotate(rightLegMat, glm::radians(personRightLegX), glm::vec3(1.0f, 0.0f, 0.0f));
            rightLegMat = glm::translate(rightLegMat, -pivotRightLeg);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(rightLegMat));
            personRightLeg->Draw(shader);

            // Pierna izquierda
            glm::vec3 pivotLeftLeg = glm::vec3(-0.06f, 1.02f, 0.03f);
            glm::mat4 leftLegMat = personBase;
            leftLegMat = glm::translate(leftLegMat, pivotLeftLeg);
            leftLegMat = glm::rotate(leftLegMat, glm::radians(personLeftLegX), glm::vec3(1.0f, 0.0f, 0.0f));
            leftLegMat = glm::translate(leftLegMat, -pivotLeftLeg);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(leftLegMat));
            personLeftLeg->Draw(shader);
        }



        glfwSwapBuffers(window);
    }

    for (Model* s : stands) delete s;
    stands.clear();
    delete birdBody;
    delete birdHead;
    delete birdWingR;
    delete birdWingRT;
    delete birdWingL;
    delete birdWingLT;
    delete birdTail;

    delete personBody;
    delete personRightArm;
    delete personLeftArm;
    delete personRightLeg;
    delete personLeftLeg;

    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);

    ///extra stands
    if (selectedStand >= 0) {
        Stand& s = standConfigs[selectedStand];
        float moveSpeed = 5.0f * deltaTime;
        float rotSpeed = 90.0f * deltaTime;
        float scaleSpeed = 1.0f * deltaTime;

        // Trasladar con flechas + PG_UP/DOWN
        if (keys[GLFW_KEY_RIGHT])     s.pos.x += moveSpeed;
        if (keys[GLFW_KEY_LEFT])      s.pos.x -= moveSpeed;
        if (keys[GLFW_KEY_UP])        s.pos.z -= moveSpeed;
        if (keys[GLFW_KEY_DOWN])      s.pos.z += moveSpeed;
        if (keys[GLFW_KEY_PAGE_UP])   s.pos.y += moveSpeed;
        if (keys[GLFW_KEY_PAGE_DOWN]) s.pos.y -= moveSpeed;

        // Rotar con Q/E
        if (keys[GLFW_KEY_Q]) s.rotY -= rotSpeed;
        if (keys[GLFW_KEY_E]) s.rotY += rotSpeed;

        // Escalar con Z/X
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

	// Selección EXTRA de stands con números 1-8, 0 para deseleccionar
    if (key == GLFW_KEY_1) selectedStand = 0;
    if (key == GLFW_KEY_2) selectedStand = 1;
    if (key == GLFW_KEY_3) selectedStand = 2;
    if (key == GLFW_KEY_4) selectedStand = 3;
    if (key == GLFW_KEY_5) selectedStand = 4;
    if (key == GLFW_KEY_6) selectedStand = 5;
    if (key == GLFW_KEY_7) selectedStand = 6;
    if (key == GLFW_KEY_8) selectedStand = 7;
    if (key == GLFW_KEY_0) selectedStand = -1;  // deseleccionar

    // Ocultar/mostrar con H
    if (key == GLFW_KEY_H && selectedStand >= 0)
        standConfigs[selectedStand].visible = !standConfigs[selectedStand].visible;

    // Ocultar/mostrar la persona con P
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        personVisible = !personVisible;
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) { lastX = xPos; lastY = yPos; firstMouse = false; }
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    lastX = xPos; lastY = yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}