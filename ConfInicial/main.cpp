// Proyecto Final - IXANIK
// Integrantes:
// 319323290
// 320260366
// 320110450

//Entrega: 13 de mayo, 2025
#include <GL/glew.h>
#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"
#include <string>
#include <vector>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "modelAnim.h"
#include "meshAnim.h"

const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

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
    { "Models/Stands/s1/principal.obj", glm::vec3(8.0f,  0.0f,  25.5f), glm::vec3(1.05f), 0.0f,  true },
    { "Models/Stands/s2/s2.obj",        glm::vec3(-9.0f, 0.0f,   4.4f), glm::vec3(0.9f),  90.0f, true },
    { "Models/Stands/s3/s3.obj",        glm::vec3(5.7f,  0.0f,   9.2f), glm::vec3(0.9f),  90.0f, true },
    { "Models/Stands/s4/s4.obj",        glm::vec3(5.7f,  0.0f,  14.0f), glm::vec3(0.01f), 90.0f, true },
    { "Models/Stands/s5/s5.obj",        glm::vec3(5.9f,  0.0f,  -1.0f), glm::vec3(0.9f),  90.0f, true },
    { "Models/Stands/s6/s6.obj",        glm::vec3(5.7f,  0.0f,  -7.0f), glm::vec3(0.85f), 0.0f,  true },
    { "Models/Stands/s7/s7.obj",        glm::vec3(5.7f,  0.0f, -11.0f), glm::vec3(0.8f),  0.0f,  true },
    { "Models/Stands/s8/s8.obj",        glm::vec3(5.7f,  0.0f, -16.0f), glm::vec3(1.0f),  0.0f,  true },
};

std::vector<Model*> stands;
int selectedStand = -1;

// ============================================================
// PÁJARO
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

glm::vec3 birdPos = glm::vec3(9.0f, 3.9f, 25.0f);
const glm::vec3 BIRD_START = glm::vec3(9.0f, 3.9f, 25.0f);
const glm::vec3 BIRD_END = glm::vec3(9.0f, 3.9f, -30.0f);
const float BIRD_SPEED = 3.0f;
bool birdVisible = true;
bool birdMoving = true;
bool sqMoving = true;

float wingRightAngle = 0.0f;
float wingLeftAngle = 0.0f;

#define BIRD_MAX_FRAMES 3
typedef struct {
    float wingAngle;
    float wingAngleInc;
} BirdWingFrame;

BirdWingFrame birdFrames[BIRD_MAX_FRAMES];
int birdMaxSteps = 10;
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
        if (fabsf(diff) < 1.0f) {
            birdFacingAngle = birdTargetAngle;
            birdBankAngle = 0.0f;
            birdTurning = false;
        }
        birdPos.z += birdDirZ * BIRD_SPEED * 0.3f * deltaTime;
        birdPos.y = BIRD_START.y;
        AnimateBirdWings();
        return;
    }
    if (birdMoving) {
        birdPos.z += birdDirZ * BIRD_SPEED * deltaTime;
        birdPos.y = BIRD_START.y + 0.3f * sinf((float)glfwGetTime() * 2.0f);
        birdPos.x = BIRD_START.x + 0.4f * sinf((float)glfwGetTime() * 0.7f);
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
    }
    AnimateBirdWings();
}

// ============================================================
// PERSONA - ANIMACIÓN POR KEYFRAMES
// ============================================================
Model* personBody = nullptr;
Model* personHead = nullptr;
Model* personRightArm = nullptr;
Model* personLeftArm = nullptr;
Model* personRightLeg = nullptr;
Model* personLeftLeg = nullptr;
bool   personVisible = true;
Model* jointMarker = nullptr;
bool showPersonJoints = true;

const glm::vec3 PERSON_STAND_POS = glm::vec3(11.0f, 0.0f, 25.0f);
const glm::vec3 PERSON_LOOK_STAND = glm::vec3(8.0f, 0.0f, 25.5f);
glm::vec3 personScale = glm::vec3(1.0f);
const float PERSON_MODEL_FORWARD_OFFSET = 160.0f;

glm::vec3 personPos = PERSON_STAND_POS;
float personYaw = 0.0f;
float personBodySideLean = 0.0f;
float personBodyForwardLean = 0.0f;
float personBodyBob = 0.0f;
float personHeadYaw = 0.0f;
float personHeadPitch = 0.0f;
float personRightArmX = 0.0f;
float personRightArmY = 0.0f;
float personRightArmZ = -75.0f;
float personLeftArmX = 0.0f;
float personLeftArmY = 0.0f;
float personLeftArmZ = 75.0f;
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
float LerpFloat(float a, float b, float t) { return a + (b - a) * t; }
float NormalizarAngulo(float angulo) {
    while (angulo > 180.0f) angulo -= 360.0f;
    while (angulo < -180.0f) angulo += 360.0f;
    return angulo;
}
float YawHaciaPunto(glm::vec3 desde, glm::vec3 hacia) {
    glm::vec3 dir = hacia - desde;
    return glm::degrees(atan2f(dir.x, dir.z)) + PERSON_MODEL_FORWARD_OFFSET;
}

struct PersonPose {
    float bodySideLean, bodyForwardLean, bodyBob;
    float headYaw, headPitch;
    float rightArmX, rightArmY, rightArmZ;
    float leftArmX, leftArmY, leftArmZ;
    float rightLegX, leftLegX;
};

PersonPose LerpPose(const PersonPose& a, const PersonPose& b, float t) {
    t = SmoothStep(t);
    PersonPose r;
    r.bodySideLean = LerpFloat(a.bodySideLean, b.bodySideLean, t);
    r.bodyForwardLean = LerpFloat(a.bodyForwardLean, b.bodyForwardLean, t);
    r.bodyBob = LerpFloat(a.bodyBob, b.bodyBob, t);
    r.headYaw = LerpFloat(a.headYaw, b.headYaw, t);
    r.headPitch = LerpFloat(a.headPitch, b.headPitch, t);
    r.rightArmX = LerpFloat(a.rightArmX, b.rightArmX, t);
    r.rightArmY = LerpFloat(a.rightArmY, b.rightArmY, t);
    r.rightArmZ = LerpFloat(a.rightArmZ, b.rightArmZ, t);
    r.leftArmX = LerpFloat(a.leftArmX, b.leftArmX, t);
    r.leftArmY = LerpFloat(a.leftArmY, b.leftArmY, t);
    r.leftArmZ = LerpFloat(a.leftArmZ, b.leftArmZ, t);
    r.rightLegX = LerpFloat(a.rightLegX, b.rightLegX, t);
    r.leftLegX = LerpFloat(a.leftLegX, b.leftLegX, t);
    return r;
}

void ApplyPersonPose(const PersonPose& p) {
    personBodySideLean = p.bodySideLean;
    personBodyForwardLean = p.bodyForwardLean;
    personBodyBob = p.bodyBob;
    personHeadYaw = p.headYaw;
    personHeadPitch = p.headPitch;
    personRightArmX = p.rightArmX;
    personRightArmY = p.rightArmY;
    personRightArmZ = p.rightArmZ;
    personLeftArmX = p.leftArmX;
    personLeftArmY = p.leftArmY;
    personLeftArmZ = p.leftArmZ;
    personRightLegX = p.rightLegX;
    personLeftLegX = p.leftLegX;
}

void UpdatePersonAnimation() {
    personAnimTime += deltaTime;
    personPos = PERSON_STAND_POS;
    personYaw = YawHaciaPunto(PERSON_STAND_POS, PERSON_LOOK_STAND) + 8.0f;

    // ---- Poses ----
    PersonPose restPose = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f, -75.0f,
        0.0f, 0.0f,  75.0f,
        0.0f, 0.0f
    };

    // Señala: brazo a 45° (natural, no exagerado)
    PersonPose pointPose = restPose;
    pointPose.bodyForwardLean = -1.5f;
    pointPose.headYaw = -6.0f;   // mira hacia el stand
    pointPose.headPitch = -3.0f;   // ligero tilt hacia abajo
    pointPose.rightArmX = 45.0f;  // antes 70 — mucho más natural
    pointPose.rightArmY = -5.0f;
    pointPose.rightArmZ = -55.0f;

    // Brazos cruzados
    PersonPose crossPose = restPose;
    crossPose.bodyForwardLean = -1.5f;
    crossPose.rightArmX = 125.0f;
    crossPose.rightArmY = -55.0f;
    crossPose.rightArmZ = -10.0f;
    crossPose.leftArmX = 125.0f;
    crossPose.leftArmY = 55.0f;
    crossPose.leftArmZ = 10.0f;

    // NUEVA POSE: mano al mentón (pensando)
    // El brazo derecho sube hasta el nivel del cuello,
    // el izquierdo lo sostiene por debajo (codo apoyado)
    PersonPose thinkPose = restPose;
    thinkPose.bodyForwardLean = 0.5f;
    thinkPose.bodySideLean = 2.0f;   // leve inclinación lateral pensativo
    thinkPose.headYaw = -8.0f;   // mira ligeramente al stand
    thinkPose.headPitch = 5.0f;   // cabeza un poco hacia abajo (reflexivo)
    thinkPose.rightArmX = 90.0f;  // brazo derecho sube al cuello
    thinkPose.rightArmY = -25.0f;
    thinkPose.rightArmZ = -20.0f;
    thinkPose.leftArmX = 80.0f;  // brazo izquierdo sostiene el codo
    thinkPose.leftArmY = 30.0f;
    thinkPose.leftArmZ = 15.0f;
    thinkPose.rightLegX = -3.0f;   // peso en pierna derecha
    thinkPose.leftLegX = 4.0f;

    // ---- Tiempos del ciclo ----
    const float REST_TIME = 0.8f;  // reposo inicial visible
    const float ARM_UP_TIME = 1.2f;  // sube brazo para señalar
    const float POINT_HOLD_TIME = 2.0f;  // mantiene señalando
    const float ARM_DOWN_TIME = 1.0f;  // baja el brazo
    const float CROSS_TIME = 1.2f;  // cruza brazos
    const float BREATH_TIME = 2.0f;  // respira cruzado
    const float HEAD_SHAKE_TIME = 2.2f;  // mueve cabeza
    const float HEAD_CENTER_TIME = 0.7f;  // centra cabeza
    const float THINK_UP_TIME = 1.0f;  // lleva mano al mentón
    const float THINK_HOLD_TIME = 2.5f;  // mantiene pose pensando
    const float THINK_DOWN_TIME = 1.0f;  // baja mano
    const float UNCROSS_TIME = 1.1f;  // descruza
    const float PAUSE_TIME = 1.0f;  // pausa final

    const float CYCLE_TIME =
        REST_TIME + ARM_UP_TIME + POINT_HOLD_TIME + ARM_DOWN_TIME +
        CROSS_TIME + BREATH_TIME + HEAD_SHAKE_TIME + HEAD_CENTER_TIME +
        THINK_UP_TIME + THINK_HOLD_TIME + THINK_DOWN_TIME +
        UNCROSS_TIME + PAUSE_TIME;

    float t = fmodf(personAnimTime, CYCLE_TIME);

    // 1) Reposo inicial
    if (t < REST_TIME) {
        float wave = sinf((float)glfwGetTime() * 1.2f);
        PersonPose pose = restPose;
        pose.bodyBob = wave * 0.008f;  // respiración sutil en reposo
        pose.bodySideLean = wave * 0.2f;
        ApplyPersonPose(pose);
        return;
    }
    t -= REST_TIME;

    // 2) Sube el brazo para señalar
    if (t < ARM_UP_TIME) {
        ApplyPersonPose(LerpPose(restPose, pointPose, t / ARM_UP_TIME));
        return;
    }
    t -= ARM_UP_TIME;

    // 3) Mantiene señalando con micro-temblor orgánico
    if (t < POINT_HOLD_TIME) {
        PersonPose pose = pointPose;
        float osc = sinf((float)glfwGetTime() * 1.6f);
        pose.rightArmX += osc * 1.8f;   // temblor leve del brazo
        pose.bodyForwardLean += osc * 0.3f;   // peso leve
        pose.headYaw += sinf((float)glfwGetTime() * 0.5f) * 1.5f; // cabeza levemente viva
        ApplyPersonPose(pose);
        return;
    }
    t -= POINT_HOLD_TIME;

    // 4) Baja el brazo
    if (t < ARM_DOWN_TIME) {
        ApplyPersonPose(LerpPose(pointPose, restPose, t / ARM_DOWN_TIME));
        return;
    }
    t -= ARM_DOWN_TIME;

    // 5) Cruza brazos
    if (t < CROSS_TIME) {
        ApplyPersonPose(LerpPose(restPose, crossPose, t / CROSS_TIME));
        return;
    }
    t -= CROSS_TIME;

    // 6) Respira con brazos cruzados
    if (t < BREATH_TIME) {
        PersonPose pose = crossPose;
        float wave = sinf((float)glfwGetTime() * 2.2f);
        pose.bodyBob = wave * 0.03f;
        pose.bodySideLean = 1.5f + wave * 1.2f;
        pose.headPitch = wave * 1.8f;
        pose.rightLegX = -2.5f;
        pose.leftLegX = 5.0f + wave * 2.5f;
        ApplyPersonPose(pose);
        return;
    }
    t -= BREATH_TIME;

    // 7) Mueve la cabeza de lado a lado
    if (t < HEAD_SHAKE_TIME) {
        PersonPose pose = crossPose;
        float p = t / HEAD_SHAKE_TIME;
        float wave = sinf((float)glfwGetTime() * 2.2f);
        pose.bodyBob = wave * 0.018f;
        pose.bodySideLean = wave * 0.7f;
        pose.headYaw = sinf(p * 6.2831853f * 1.5f) * 16.0f;
        ApplyPersonPose(pose);
        return;
    }
    t -= HEAD_SHAKE_TIME;

    // 8) Centra la cabeza
    if (t < HEAD_CENTER_TIME) {
        PersonPose headSide = crossPose;
        headSide.headYaw = 16.0f;
        ApplyPersonPose(LerpPose(headSide, crossPose, t / HEAD_CENTER_TIME));
        return;
    }
    t -= HEAD_CENTER_TIME;

    // 9) Lleva mano al mentón (pensando)
    if (t < THINK_UP_TIME) {
        ApplyPersonPose(LerpPose(crossPose, thinkPose, t / THINK_UP_TIME));
        return;
    }
    t -= THINK_UP_TIME;

    // 10) Mantiene pose pensando con movimiento muy sutil de cabeza
    if (t < THINK_HOLD_TIME) {
        PersonPose pose = thinkPose;
        float osc = sinf((float)glfwGetTime() * 0.8f);
        // Cabeza se mueve levemente como si estuviera pensando
        pose.headYaw += osc * 4.0f;
        pose.headPitch += fabsf(sinf((float)glfwGetTime() * 0.4f)) * 2.0f;
        // Leve balanceo del torso
        pose.bodySideLean += sinf((float)glfwGetTime() * 1.1f) * 0.8f;
        // Micro-movimiento del brazo en el mentón
        pose.rightArmX += sinf((float)glfwGetTime() * 2.0f) * 1.5f;
        ApplyPersonPose(pose);
        return;
    }
    t -= THINK_HOLD_TIME;

    // 11) Baja la mano del mentón de vuelta a reposo
    if (t < THINK_DOWN_TIME) {
        ApplyPersonPose(LerpPose(thinkPose, restPose, t / THINK_DOWN_TIME));
        return;
    }
    t -= THINK_DOWN_TIME;

    // 12) Descruza (ya viene de restPose así que es suave)
    if (t < UNCROSS_TIME) {
        ApplyPersonPose(LerpPose(restPose, restPose, t / UNCROSS_TIME));
        return;
    }
    t -= UNCROSS_TIME;

    // 13) Pausa final antes de repetir
    ApplyPersonPose(restPose);
}

// ============================================================
// ARDILLA
// ============================================================
Model* sqBody = nullptr;
Model* sqLeg1 = nullptr;
Model* sqLeg2 = nullptr;
Model* sqArm1 = nullptr;
Model* sqArm2 = nullptr;
Model* sqTail = nullptr;
bool sqVisible = true;

std::vector<glm::vec3> sqPath = {
    glm::vec3(15.0f, 0.5f,  25.0f),
    glm::vec3(9.5f,  0.5f,  20.0f),
    glm::vec3(9.5f,  0.5f,   0.0f),
    glm::vec3(9.5f,  0.5f, -15.0f),
    glm::vec3(5.0f,  0.5f, -20.0f)
};
int currentWP = 0;
float sqLerpT = 0.0f;
glm::vec3 sqPos = sqPath[0];
float sqYaw = 0.0f;
float sqWalkAngle = 0.0f;

Shader* animShader = nullptr;
ModelAnim* animacionPersonaje = nullptr;

void UpdateSquirrel() {
    if (sqMoving) {
        float sqSpeed = 0.5f;
        sqLerpT += sqSpeed * deltaTime;
        int nextWP = currentWP + 1;
        if (sqLerpT > 1.0f) {
            sqLerpT = 0.0f;
            currentWP++;
            nextWP++;
            if (currentWP >= (int)sqPath.size() - 1) {
                currentWP = 0;
                nextWP = 1;
            }
        }
        glm::vec3 pStart = sqPath[currentWP];
        glm::vec3 pEnd = sqPath[nextWP];
        sqPos = pStart + sqLerpT * (pEnd - pStart);
        glm::vec3 sqDir = glm::normalize(pEnd - pStart);
        sqYaw = glm::degrees(atan2f(sqDir.x, sqDir.z));
        sqPos.y = 0.5f + fabsf(sinf((float)glfwGetTime() * 15.0f)) * 0.3f;
    }
    sqWalkAngle = sinf((float)glfwGetTime() * 15.0f) * 25.0f;
}

GLuint CargarTexturaDesdeCodigo(const char* ruta) {
    GLuint texturaID;
    glGenTextures(1, &texturaID);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(ruta, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, texturaID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "ERROR: No se pudo cargar la textura: " << ruta << std::endl;
    }
    stbi_image_free(data);
    return texturaID;
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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    for (int i = 0; i < (int)standConfigs.size(); i++)
        stands.push_back(new Model((char*)standConfigs[i].path.c_str()));

    // ---- Letreros ----
    Model AulaMagna((char*)"Models/Letreros/modelo_cartel_aula_magna.obj");
    Model PlacaFlecha((char*)"Models/Letreros/modelo_cartel_verde_flecha.obj");
    Model CartelAzul((char*)"Models/Letreros/modelo_cartel_azul.obj");
    Model UnionProf((char*)"Models/Letreros/modelo_cartel_union_profesores.obj");
    Model UnionProfSA((char*)"Models/Letreros/modelo_cartel_union_profesores_sa.obj");
    Model ConsejoTecnico((char*)"Models/Letreros/modelo_cartel_consejo_tecnico.obj");
    Model SSA((char*)"Models/Letreros/modelo_cartel_ssa.obj");
    Model DCSyH((char*)"Models/Letreros/modelo_cartel_dscyh.obj");
    Model ZonaRiesgo((char*)"Models/Letreros/modelo_cartel_zona_riesgo.obj");
    Model Lona((char*)"Models/Letreros/cartel_lona.obj");
    Model CartelesInfo((char*)"Models/Letreros/modelo_carteles_informativos.obj");

    // ---- Pantalla dinámica ----
    Model Pantalla((char*)"Models/PantallaDinamica/pantalla.obj");
    const int NUM_TEXTURAS_OBJETO = 4;
    GLuint texturasObjeto[NUM_TEXTURAS_OBJETO];
    texturasObjeto[0] = CargarTexturaDesdeCodigo("Models/PantallaDinamica/Texturas/img/bienvenida.jpg");
    texturasObjeto[1] = CargarTexturaDesdeCodigo("Models/PantallaDinamica/Texturas/img/IA.jpg");
    texturasObjeto[2] = CargarTexturaDesdeCodigo("Models/PantallaDinamica/Texturas/img/redes.jpg");
    texturasObjeto[3] = CargarTexturaDesdeCodigo("Models/PantallaDinamica/Texturas/img/robotica.jpg");

    // ---- Pájaro ----
    birdBody = new Model((char*)"Models/Bird/cuerpo.obj");
    birdHead = new Model((char*)"Models/Bird/cabeza.obj");
    birdWingR = new Model((char*)"Models/Bird/alaDer.obj");
    birdWingRT = new Model((char*)"Models/Bird/alaDer_Punta.obj");
    birdWingL = new Model((char*)"Models/Bird/alaIzq.obj");
    birdWingLT = new Model((char*)"Models/Bird/alaIzq_punt.obj");
    birdTail = new Model((char*)"Models/Bird/cola.obj");

    // ---- Persona jerárquica ----
    personBody = new Model((char*)"Models/Persona/persona_cuerpo_sin_cabeza.obj");
    personHead = new Model((char*)"Models/Persona/persona_cabeza.obj");
    personRightArm = new Model((char*)"Models/Persona/persona_brazo_derecho.obj");
    personLeftArm = new Model((char*)"Models/Persona/persona_brazo_izquierdo.obj");
    personRightLeg = new Model((char*)"Models/Persona/persona_pierna_derecha.obj");
    personLeftLeg = new Model((char*)"Models/Persona/persona_pierna_izquierda.obj");
    jointMarker = new Model((char*)"Models/Persona/articulacion.obj");

    // ---- Ardilla ----
    sqBody = new Model((char*)"Models/ardilla/cuerpo.obj");
    sqLeg1 = new Model((char*)"Models/ardilla/pata1.obj");
    sqLeg2 = new Model((char*)"Models/ardilla/pata2.obj");
    sqArm1 = new Model((char*)"Models/ardilla/mano1.obj");
    sqArm2 = new Model((char*)"Models/ardilla/mano2.obj");
    sqTail = new Model((char*)"Models/ardilla/cola.obj");

    // ---- Animación esquelética ----
    animShader = new Shader("Shader/anim.vs", "Shader/anim.frag");
    animacionPersonaje = new ModelAnim("Models/persona1/persona1.fbx");
    animacionPersonaje->initShaders(animShader->Program);

    // ---- Textura Ixanik ----
    unsigned int texturaIxanik;
    glGenTextures(1, &texturaIxanik);
    glBindTexture(GL_TEXTURE_2D, texturaIxanik);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    {
        int w, h, ch;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load("Models/persona1/p3hero.png", &w, &h, &ch, 0);
        if (data) {
            GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            printf("ERROR: No se encontro p3hero.png.\n");
        }
        stbi_image_free(data);
    }

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

    glm::vec3 posPersona = glm::vec3(9.1f, 0.0f, -46.0f);
    float     anglePersona = 0.0f;
    int       estadoPersona = 0;

    glm::vec3 posPersona2 = glm::vec3(10.0f, 0.0f, 22.0f);
    float     anglePersona2 = 180.0f;
    int       estadoPersona2 = 2;

    lastFrame = (GLfloat)glfwGetTime();

    // ============================================================
    // LOOP PRINCIPAL
    // ============================================================
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        UpdateBird();
        UpdatePersonAnimation();
        UpdateSquirrel();

        // ---- Lógica recorrido persona 1 ----
        float velocidadPersona = 4.0f * deltaTime;
        if (estadoPersona == 0) {
            posPersona.z += velocidadPersona;
            anglePersona = 0.0f;
            if (posPersona.z >= 22.0f) { posPersona.z = 22.0f;  estadoPersona = 2; }
        }
        else if (estadoPersona == 2) {
            posPersona.z -= velocidadPersona;
            anglePersona = 180.0f;
            if (posPersona.z <= -46.0f) { posPersona.z = -46.0f; estadoPersona = 0; }
        }

        // ---- Lógica recorrido persona 2 (clon) ----
        if (estadoPersona2 == 0) {
            posPersona2.z += velocidadPersona;
            anglePersona2 = 0.0f;
            if (posPersona2.z >= 22.0f) { posPersona2.z = 22.0f;  estadoPersona2 = 2; }
        }
        else if (estadoPersona2 == 2) {
            posPersona2.z -= velocidadPersona;
            anglePersona2 = 180.0f;
            if (posPersona2.z <= -46.0f) { posPersona2.z = -46.0f; estadoPersona2 = 0; }
        }

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();
        glUniform1i(glGetUniformLocation(shader.Program, "usarTexturaForzada"), 0);

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

        // ---- Lobby ----
        glm::mat4 modelLobby = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelLobby));
        lobby.Draw(shader);

        // ---- Stands ----
        for (int i = 0; i < (int)stands.size(); i++) {
            if (!standConfigs[i].visible) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, standConfigs[i].pos);
            model = glm::rotate(model, glm::radians(standConfigs[i].rotY), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, standConfigs[i].scale);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            stands[i]->Draw(shader);
        }

        // ---- Pantalla dinámica ----
        {
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(11.7f, 2.0f, 18.1f));
            m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::scale(m, glm::vec3(1.2f));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m));
            int texturaActual = ((int)((float)glfwGetTime() * 0.8f)) % NUM_TEXTURAS_OBJETO;
            glUniform1i(glGetUniformLocation(shader.Program, "usarTexturaForzada"), 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texturasObjeto[texturaActual]);
            glUniform1i(glGetUniformLocation(shader.Program, "texturaForzada"), 0);
            Pantalla.Draw(shader);
            glUniform1i(glGetUniformLocation(shader.Program, "usarTexturaForzada"), 0);
        }

        // ---- Letreros ----
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(11.7f, 2.16f, 13.58f)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); CartelesInfo.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(11.75f, 2.2f, 0.0f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); AulaMagna.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(11.75f, 3.3f, 13.5f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); PlacaFlecha.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(11.75f, 3.5f, 13.5f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); CartelAzul.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(10.78f, 2.8f, 9.7f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); UnionProf.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(11.7f, 2.35f, 6.9f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); UnionProfSA.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(11.7f, 2.35f, -4.5f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); ConsejoTecnico.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(11.7f, 2.35f, -11.0f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); SSA.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(11.7f, 2.35f, -17.0f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); DCSyH.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(11.75f, 2.5f, 0.0f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); PlacaFlecha.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(10.8f, 3.0f, -22.8f)); m = glm::scale(m, glm::vec3(1.5f)); m = glm::rotate(m, glm::radians(270.0f), glm::vec3(1, 0, 0)); m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); ZonaRiesgo.Draw(shader); }
        { glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(10.8f, 1.0f, -18.0f)); m = glm::rotate(m, glm::radians(120.0f), glm::vec3(0, 1, 0)); m = glm::scale(m, glm::vec3(2.0f)); glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m)); Lona.Draw(shader); }

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

            float headYaw = sinf((float)glfwGetTime() * 0.9f) * 7.0f;
            float headPitch = -wingRightAngle * 0.18f;
            glm::mat4 headMat = birdBase;
            headMat = glm::translate(headMat, glm::vec3(1.651f, -0.286f, -0.345f));
            headMat = glm::rotate(headMat, glm::radians(headYaw), glm::vec3(0, 1, 0));
            headMat = glm::rotate(headMat, glm::radians(headPitch), glm::vec3(1, 0, 0));
            headMat = glm::translate(headMat, -glm::vec3(1.651f, -0.286f, -0.345f));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(headMat));
            birdHead->Draw(shader);

            float tailPitch = wingRightAngle * 0.22f;
            glm::mat4 tailMat = birdBase;
            tailMat = glm::translate(tailMat, glm::vec3(0.389f, -1.266f, -0.310f));
            tailMat = glm::rotate(tailMat, glm::radians(tailPitch), glm::vec3(1, 0, 0));
            tailMat = glm::translate(tailMat, -glm::vec3(0.389f, -1.266f, -0.310f));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(tailMat));
            birdTail->Draw(shader);

            glm::vec3 pivotR = glm::vec3(1.05f, -0.66f, 0.0f);
            glm::mat4 wingRRoot = birdBase;
            wingRRoot = glm::translate(wingRRoot, pivotR);
            wingRRoot = glm::rotate(wingRRoot, glm::radians(wingRightAngle), glm::vec3(1, 0, 0));
            wingRRoot = glm::translate(wingRRoot, -pivotR);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingRRoot));
            birdWingR->Draw(shader);

            glm::vec3 pivotRT = glm::vec3(1.05f, -0.66f, 0.44f);
            float tipFoldR = wingRightAngle * 0.7f - 10.0f;
            glm::mat4 wingRTip = wingRRoot;
            wingRTip = glm::translate(wingRTip, pivotRT);
            wingRTip = glm::rotate(wingRTip, glm::radians(tipFoldR), glm::vec3(1, 0, 0));
            wingRTip = glm::translate(wingRTip, -pivotRT);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingRTip));
            birdWingRT->Draw(shader);

            glm::vec3 pivotL = glm::vec3(1.15f, -0.66f, -0.33f);
            glm::mat4 wingLRoot = birdBase;
            wingLRoot = glm::translate(wingLRoot, pivotL);
            wingLRoot = glm::rotate(wingLRoot, glm::radians(-wingRightAngle), glm::vec3(1, 0, 0));
            wingLRoot = glm::translate(wingLRoot, -pivotL);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wingLRoot));
            birdWingL->Draw(shader);

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
        // RENDERIZAR PERSONA JERÁRQUICA
        // ============================================================
        if (personVisible && personBody && personHead && personRightArm
            && personLeftArm && personRightLeg && personLeftLeg)
        {
            glm::mat4 personBase = glm::mat4(1.0f);
            personBase = glm::translate(personBase, personPos + glm::vec3(0.0f, personBodyBob, 0.0f));
            personBase = glm::rotate(personBase, glm::radians(personYaw), glm::vec3(0, 1, 0));
            personBase = glm::rotate(personBase, glm::radians(personBodyForwardLean), glm::vec3(1, 0, 0));
            personBase = glm::rotate(personBase, glm::radians(personBodySideLean), glm::vec3(0, 0, 1));
            personBase = glm::scale(personBase, personScale);

            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(personBase));
            personBody->Draw(shader);

            glm::vec3 pivotHead = glm::vec3(0.02f, 1.56f, 0.06f);
            glm::mat4 headMat = personBase;
            headMat = glm::translate(headMat, pivotHead);
            headMat = glm::rotate(headMat, glm::radians(personHeadYaw), glm::vec3(0, 1, 0));
            headMat = glm::rotate(headMat, glm::radians(personHeadPitch), glm::vec3(1, 0, 0));
            headMat = glm::translate(headMat, -pivotHead);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(headMat));
            personHead->Draw(shader);

            glm::vec3 pivotRightArm = glm::vec3(0.13f, 1.45f, 0.08f);
            glm::mat4 rightArmMat = personBase;
            rightArmMat = glm::translate(rightArmMat, pivotRightArm);
            rightArmMat = glm::rotate(rightArmMat, glm::radians(personRightArmX), glm::vec3(1, 0, 0));
            rightArmMat = glm::rotate(rightArmMat, glm::radians(personRightArmY), glm::vec3(0, 1, 0));
            rightArmMat = glm::rotate(rightArmMat, glm::radians(personRightArmZ), glm::vec3(0, 0, 1));
            rightArmMat = glm::translate(rightArmMat, -pivotRightArm);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(rightArmMat));
            personRightArm->Draw(shader);

            glm::vec3 pivotLeftArm = glm::vec3(-0.13f, 1.45f, 0.08f);
            glm::mat4 leftArmMat = personBase;
            leftArmMat = glm::translate(leftArmMat, pivotLeftArm);
            leftArmMat = glm::rotate(leftArmMat, glm::radians(personLeftArmX), glm::vec3(1, 0, 0));
            leftArmMat = glm::rotate(leftArmMat, glm::radians(personLeftArmY), glm::vec3(0, 1, 0));
            leftArmMat = glm::rotate(leftArmMat, glm::radians(personLeftArmZ), glm::vec3(0, 0, 1));
            leftArmMat = glm::translate(leftArmMat, -pivotLeftArm);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(leftArmMat));
            personLeftArm->Draw(shader);

            glm::vec3 pivotRightLeg = glm::vec3(0.08f, 0.92f, 0.03f);
            glm::mat4 rightLegMat = personBase;
            rightLegMat = glm::translate(rightLegMat, pivotRightLeg);
            rightLegMat = glm::rotate(rightLegMat, glm::radians(personRightLegX), glm::vec3(1, 0, 0));
            rightLegMat = glm::translate(rightLegMat, -pivotRightLeg);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(rightLegMat));
            personRightLeg->Draw(shader);

            glm::vec3 pivotLeftLeg = glm::vec3(-0.08f, 0.92f, 0.03f);
            glm::mat4 leftLegMat = personBase;
            leftLegMat = glm::translate(leftLegMat, pivotLeftLeg);
            leftLegMat = glm::rotate(leftLegMat, glm::radians(personLeftLegX), glm::vec3(1, 0, 0));
            leftLegMat = glm::translate(leftLegMat, -pivotLeftLeg);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(leftLegMat));
            personLeftLeg->Draw(shader);

            if (showPersonJoints && jointMarker) {
                auto DrawJoint = [&](glm::vec3 pivot, float size) {
                    glm::mat4 jm = personBase;
                    jm = glm::translate(jm, pivot + glm::vec3(0.0f, 0.045f, 0.0f));
                    jm = glm::scale(jm, glm::vec3(size));
                    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(jm));
                    jointMarker->Draw(shader);
                    };
                DrawJoint(pivotHead, 0.06f);
                DrawJoint(pivotRightArm, 0.37f);
                DrawJoint(pivotLeftArm, 0.37f);
                DrawJoint(pivotRightLeg, 0.06f);
                DrawJoint(pivotLeftLeg, 0.06f);
            }
        }

        // ============================================================
        // RENDERIZAR ARDILLA
        // ============================================================
        if (sqVisible) {
            glm::mat4 sqBase = glm::mat4(1.0f);
            sqBase = glm::translate(sqBase, sqPos);
            sqBase = glm::rotate(sqBase, glm::radians(sqYaw), glm::vec3(0, 1, 0));
            sqBase = glm::scale(sqBase, glm::vec3(0.2f));
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(sqBase));
            sqBody->Draw(shader);

            glm::vec3 pivotCola = glm::vec3(0.2569f, -0.12934f, -0.21342f);
            float anguloCola = sinf((float)glfwGetTime() * 5.0f) * 15.0f;
            glm::mat4 colaMat = sqBase;
            colaMat = glm::translate(colaMat, pivotCola);
            colaMat = glm::rotate(colaMat, glm::radians(anguloCola), glm::vec3(1, 0, 0));
            colaMat = glm::translate(colaMat, -pivotCola);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(colaMat));
            sqTail->Draw(shader);

            glm::vec3 pivotMano1 = glm::vec3(0.2569f, -0.12934f, -0.21342f);
            glm::mat4 mano1Mat = sqBase;
            mano1Mat = glm::translate(mano1Mat, pivotMano1);
            mano1Mat = glm::rotate(mano1Mat, glm::radians(sqWalkAngle), glm::vec3(1, 0, 0));
            mano1Mat = glm::translate(mano1Mat, -pivotMano1);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mano1Mat));
            sqArm1->Draw(shader);

            glm::vec3 pivotMano2 = glm::vec3(0.2569f, -0.12934f, 0.21342f);
            glm::mat4 mano2Mat = sqBase;
            mano2Mat = glm::translate(mano2Mat, pivotMano2);
            mano2Mat = glm::rotate(mano2Mat, glm::radians(-sqWalkAngle), glm::vec3(1, 0, 0));
            mano2Mat = glm::translate(mano2Mat, -pivotMano2);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mano2Mat));
            sqArm2->Draw(shader);

            glm::vec3 pivotPata1 = glm::vec3(-0.34106f, -1.1076f, -0.61203f);
            glm::mat4 pata1Mat = sqBase;
            pata1Mat = glm::translate(pata1Mat, pivotPata1);
            pata1Mat = glm::rotate(pata1Mat, glm::radians(-sqWalkAngle), glm::vec3(1, 0, 0));
            pata1Mat = glm::translate(pata1Mat, -pivotPata1);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(pata1Mat));
            sqLeg1->Draw(shader);

            glm::vec3 pivotPata2 = glm::vec3(0.70116f, -1.109f, -0.65543f);
            glm::mat4 pata2Mat = sqBase;
            pata2Mat = glm::translate(pata2Mat, pivotPata2);
            pata2Mat = glm::rotate(pata2Mat, glm::radians(sqWalkAngle), glm::vec3(1, 0, 0));
            pata2Mat = glm::translate(pata2Mat, -pivotPata2);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(pata2Mat));
            sqLeg2->Draw(shader);
        }

        // ============================================================
        // RENDERIZAR PERSONAS CON ESQUELETO
        // ============================================================
        animShader->Use();
        GLuint modelLoc = glGetUniformLocation(animShader->Program, "model");
        GLuint viewLoc = glGetUniformLocation(animShader->Program, "view");
        GLuint projLoc = glGetUniformLocation(animShader->Program, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(animShader->Program, "light.direction"), -0.3f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(animShader->Program, "light.ambient"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(animShader->Program, "light.diffuse"), 0.6f, 0.6f, 0.6f);
        glUniform3f(glGetUniformLocation(animShader->Program, "light.specular"), 0.3f, 0.3f, 0.3f);
        glUniform3f(glGetUniformLocation(animShader->Program, "material.specular"), 0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(animShader->Program, "material.shininess"), 32.0f);
        glUniform3f(glGetUniformLocation(animShader->Program, "viewPos"),
            camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        glUniform1i(glGetUniformLocation(animShader->Program, "usarForzada"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texturaIxanik);
        glUniform1i(glGetUniformLocation(animShader->Program, "texturaForzada"), 1);

        glm::mat4 modelAnim = glm::mat4(1.0f);
        modelAnim = glm::translate(modelAnim, posPersona);
        modelAnim = glm::rotate(modelAnim, glm::radians(anglePersona), glm::vec3(0, 1, 0));
        modelAnim = glm::scale(modelAnim, glm::vec3(0.013f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAnim));
        animacionPersonaje->Draw(*animShader);

        glm::mat4 modelAnim2 = glm::mat4(1.0f);
        modelAnim2 = glm::translate(modelAnim2, posPersona2);
        modelAnim2 = glm::rotate(modelAnim2, glm::radians(anglePersona2), glm::vec3(0, 1, 0));
        modelAnim2 = glm::scale(modelAnim2, glm::vec3(0.013f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAnim2));
        animacionPersonaje->Draw(*animShader);

        glUniform1i(glGetUniformLocation(animShader->Program, "usarForzada"), 0);

        glfwSwapBuffers(window);
    }

    for (Model* s : stands) delete s;
    stands.clear();
    delete birdBody; delete birdHead; delete birdWingR; delete birdWingRT;
    delete birdWingL; delete birdWingLT; delete birdTail;
    delete personBody; delete personHead; delete personRightArm; delete personLeftArm;
    delete personRightLeg; delete personLeftLeg; delete jointMarker;
    delete sqBody; delete sqLeg1; delete sqLeg2;
    delete sqArm1; delete sqArm2; delete sqTail;
    delete animacionPersonaje;
    delete animShader;
    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);

    if (selectedStand >= 0) {
        Stand& s = standConfigs[selectedStand];
        float moveSpeed = 5.0f * deltaTime;
        float rotSpeed = 90.0f * deltaTime;
        float scaleSpeed = 1.0f * deltaTime;
        if (keys[GLFW_KEY_I]) s.pos.z -= moveSpeed;
        if (keys[GLFW_KEY_K]) s.pos.z += moveSpeed;
        if (keys[GLFW_KEY_J]) s.pos.x -= moveSpeed;
        if (keys[GLFW_KEY_L]) s.pos.x += moveSpeed;
        if (keys[GLFW_KEY_U]) s.pos.y += moveSpeed;
        if (keys[GLFW_KEY_O]) s.pos.y -= moveSpeed;
        if (keys[GLFW_KEY_Q]) s.rotY -= rotSpeed;
        if (keys[GLFW_KEY_E]) s.rotY += rotSpeed;
        if (keys[GLFW_KEY_Z]) s.scale -= glm::vec3(scaleSpeed);
        if (keys[GLFW_KEY_X]) s.scale += glm::vec3(scaleSpeed);
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS) birdMoving = !birdMoving;
    if (key == GLFW_KEY_N && action == GLFW_PRESS) sqMoving = !sqMoving;
    if (key == GLFW_KEY_1) selectedStand = 0;
    if (key == GLFW_KEY_2) selectedStand = 1;
    if (key == GLFW_KEY_3) selectedStand = 2;
    if (key == GLFW_KEY_4) selectedStand = 3;
    if (key == GLFW_KEY_5) selectedStand = 4;
    if (key == GLFW_KEY_6) selectedStand = 5;
    if (key == GLFW_KEY_7) selectedStand = 6;
    if (key == GLFW_KEY_8) selectedStand = 7;
    if (key == GLFW_KEY_0) selectedStand = -1;
    if (key == GLFW_KEY_H && selectedStand >= 0 && action == GLFW_PRESS)
        standConfigs[selectedStand].visible = !standConfigs[selectedStand].visible;
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        personVisible = !personVisible;
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) { lastX = (GLfloat)xPos; lastY = (GLfloat)yPos; firstMouse = false; }
    GLfloat xOffset = (GLfloat)xPos - lastX;
    GLfloat yOffset = lastY - (GLfloat)yPos;
    lastX = (GLfloat)xPos;
    lastY = (GLfloat)yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}