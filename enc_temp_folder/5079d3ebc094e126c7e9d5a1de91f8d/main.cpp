// Proyecto Final - IXANIK
// Integrantes:
// 319323290
// 320260366
// 320110450
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
#include <iostream>
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
// PERSONA - ANIMACIÓN POR KEYFRAMES 
// ============================================================

Model* personBody = nullptr;
Model* personHead = nullptr;
Model* personRightArm = nullptr;
Model* personLeftArm = nullptr;
Model* personRightLeg = nullptr;
Model* personLeftLeg = nullptr;
bool  personVisible = true;
Model* jointMarker = nullptr;
bool showPersonJoints = true;

// Posición fija de la persona
const glm::vec3 PERSON_STAND_POS = glm::vec3(11.0f, 0.0f, 25.0f);
// Punto hacia donde mira/señala, aproximadamente el stand
const glm::vec3 PERSON_LOOK_STAND = glm::vec3(8.0f, 0.0f, 25.5f);
// Ajuste general de orientación del modelo.
glm::vec3 personScale = glm::vec3(1.0f);
const float PERSON_MODEL_FORWARD_OFFSET = 160.0f;

// Valores que se actualizan cada frame.
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

struct PersonPose {
    float bodySideLean;
    float bodyForwardLean;
    float bodyBob;

    float headYaw;
    float headPitch;

    float rightArmX;
    float rightArmY;
    float rightArmZ;

    float leftArmX;
    float leftArmY;
    float leftArmZ;

    float rightLegX;
    float leftLegX;
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

    // La persona siempre se queda fija en esta posición
    personPos = PERSON_STAND_POS;

    // La persona mira hacia el stand
    float yawLookStand = YawHaciaPunto(PERSON_STAND_POS, PERSON_LOOK_STAND);
    yawLookStand += 8.0f; // ajuste fino de orientación
    personYaw = yawLookStand;

    // Poses principales del modelo jerárquico.
    const float ARM_POINT_X = 70.0f;
    const float ARM_POINT_Y = 0.0f;
    const float ARM_POINT_Z = -65.0f;

    PersonPose restPose = {
        0.0f, 0.0f, 0.0f,      // cuerpo
        0.0f, 0.0f,            // cabeza
        0.0f, 0.0f, -75.0f,    // brazo derecho
        0.0f, 0.0f, 75.0f,     // brazo izquierdo
        0.0f, 0.0f             // piernas
    };

    PersonPose pointPose = restPose;
    pointPose.bodyForwardLean = -2.0f;
    pointPose.headYaw = -4.0f;
    pointPose.rightArmX = ARM_POINT_X;
    pointPose.rightArmY = ARM_POINT_Y;
    pointPose.rightArmZ = ARM_POINT_Z;

    PersonPose crossPose = restPose;
    crossPose.bodyForwardLean = -1.5f;
    crossPose.rightArmX = 125.0f;
    crossPose.rightArmY = -55.0f;
    crossPose.rightArmZ = -10.0f;
    crossPose.leftArmX = 125.0f;
    crossPose.leftArmY = 55.0f;
    crossPose.leftArmZ = 10.0f;

    // Tiempos del ciclo completo
    const float REST_TIME = 0.0f;
    const float ARM_UP_TIME = 1.2f;
    const float POINT_HOLD_TIME = 1.7f;
    const float ARM_DOWN_TIME = 1.1f;
    const float CROSS_TIME = 1.4f;
    const float BREATH_TIME = 2.0f;
    const float HEAD_SHAKE_TIME = 2.2f;
    const float HEAD_CENTER_TIME = 0.8f;
    const float UNCROSS_TIME = 1.3f;
    const float PAUSE_TIME = 1.0f;

    const float CYCLE_TIME = REST_TIME + ARM_UP_TIME + POINT_HOLD_TIME + ARM_DOWN_TIME +
        CROSS_TIME + BREATH_TIME + HEAD_SHAKE_TIME + HEAD_CENTER_TIME + UNCROSS_TIME + PAUSE_TIME;

    float t = fmod(personAnimTime, CYCLE_TIME);
    //SECUENCIA DE LOS MOVIMIENTOS DE LA ANIMACION 
    //1) Reposo
    if (t < REST_TIME) {
        ApplyPersonPose(restPose);
        return;
    }
    t -= REST_TIME;

    //2) Señala el stand
    if (t < ARM_UP_TIME) {
        float p = t / ARM_UP_TIME;
        ApplyPersonPose(LerpPose(restPose, pointPose, p));
        return;
    }
    t -= ARM_UP_TIME;

    //3) Mantiene el brazo señalando
    if (t < POINT_HOLD_TIME) {
        PersonPose pose = pointPose;
        pose.rightArmX += (float)sin(glfwGetTime() * 1.4f) * 1.5f;
        pose.bodyForwardLean += (float)sin(glfwGetTime() * 1.0f) * 0.4f;
        ApplyPersonPose(pose);
        return;
    }
    t -= POINT_HOLD_TIME;

    //4) Baja el brazo
    if (t < ARM_DOWN_TIME) {
        float p = t / ARM_DOWN_TIME;
        ApplyPersonPose(LerpPose(pointPose, restPose, p));
        return;
    }
    t -= ARM_DOWN_TIME;

    //5) Cruza los brazos
    if (t < CROSS_TIME) {
        float p = t / CROSS_TIME;
        ApplyPersonPose(LerpPose(restPose, crossPose, p));
        return;
    }
    t -= CROSS_TIME;

    //6) Respira y se balancea un poco con brazos cruzados
    if (t < BREATH_TIME) {
        PersonPose pose = crossPose;
        float wave = (float)sin(glfwGetTime() * 2.2f);
        //Movimiento de respiración
        pose.bodyBob = wave * 0.035f;
        //Balanceo un poco más marcado hacia un lado
        pose.bodySideLean = 1.5f + wave * 1.5f;
        //Movimiento leve de cabeza
        pose.headPitch = wave * 2.0f;
        //Simulacion de que se recarga más en una pierna
        pose.rightLegX = -2.5f;              // pierna de apoyo, casi fija
        pose.leftLegX = 6.0f + wave * 3.0f;  // pierna que se mueve poquito
        ApplyPersonPose(pose);
        return;
    }

    //7) Mueve la cabeza de lado a lado
    if (t < HEAD_SHAKE_TIME) {
        PersonPose pose = crossPose;
        float p = t / HEAD_SHAKE_TIME;
        float wave = (float)sin(glfwGetTime() * 2.2f);
        pose.bodyBob = wave * 0.020f;
        pose.bodySideLean = wave * 0.8f;
        pose.headYaw = (float)sin(p * 6.2831853f * 1.5f) * 18.0f;
        ApplyPersonPose(pose);
        return;
    }
    t -= HEAD_SHAKE_TIME;

    //8) Regresa la cabeza al centro
    if (t < HEAD_CENTER_TIME) {
        float p = t / HEAD_CENTER_TIME;
        PersonPose headSidePose = crossPose;
        headSidePose.headYaw = 18.0f;
        ApplyPersonPose(LerpPose(headSidePose, crossPose, p));
        return;
    }
    t -= HEAD_CENTER_TIME;

    //9) Mueve los brazos a la posicion de inicio
    if (t < UNCROSS_TIME) {
        float p = t / UNCROSS_TIME;
        ApplyPersonPose(LerpPose(crossPose, restPose, p));
        return;
    }
    t -= UNCROSS_TIME;

    //10) Pausa final antes de repetir
    ApplyPersonPose(restPose);
}

//============================================================ PERSONA

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


// --- Variables para el nuevo personaje animado ---
Shader* animShader;
ModelAnim* animacionPersonaje;
glm::vec3 posPersona = glm::vec3(15.0f, 0.0f, 25.0f); // Posición inicial
float anglePersona = 180.0f;
int estadoPersona = 0; // 0: Ida, 1: Pausa, 2: Vuelta, 3: Pausa
float timerPausa = 0.0f;


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
GLuint CargarTexturaDesdeCodigo(const char* ruta)
{
    GLuint texturaID;
    glGenTextures(1, &texturaID);

    int width, height, nrChannels;

    // Si la textura sale al revés, cambia true por false
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(ruta, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = GL_RGB;

        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, texturaID);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            format,
            width,
            height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            data
        );

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
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

    // Activar Depth Test para manejar la oclusión espacial de modelos 3D
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // Instanciar todos los modelos de los stands del arreglo
    for (int i = 0; i < (int)standConfigs.size(); i++)
        stands.push_back(new Model((char*)standConfigs[i].path.c_str()));

	//----------------------------------------------------
    //------ Carga de los archivos del los letreros ------
    //----------------------------------------------------
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


    //cargar archivos del modelo para la pantalla dinamica 
	Model Pantalla((char*)"Models/PantallaDinamica/pantalla.obj");

    const int NUM_TEXTURAS_OBJETO = 4;

    GLuint texturasObjeto[NUM_TEXTURAS_OBJETO];

    texturasObjeto[0] = CargarTexturaDesdeCodigo("Models/PantallaDinamica/Texturas/img/bienvenida.jpg");
    texturasObjeto[1] = CargarTexturaDesdeCodigo("Models/PantallaDinamica/Texturas/img/IA.jpg");
    texturasObjeto[2] = CargarTexturaDesdeCodigo("Models/PantallaDinamica/Texturas/img/redes.jpg");
	texturasObjeto[3] = CargarTexturaDesdeCodigo("Models/PantallaDinamica/Texturas/img/robotica.jpg");
    

    // Cargar los archivos del modelo jerárquico del pájaro
    birdBody = new Model((char*)"Models/Bird/cuerpo.obj");
    birdHead = new Model((char*)"Models/Bird/cabeza.obj");
    birdWingR = new Model((char*)"Models/Bird/alaDer.obj");
    birdWingRT = new Model((char*)"Models/Bird/alaDer_Punta.obj");
    birdWingL = new Model((char*)"Models/Bird/alaIzq.obj");
    birdWingLT = new Model((char*)"Models/Bird/alaIzq_punt.obj");
    birdTail = new Model((char*)"Models/Bird/cola.obj");

    // Cargar los archivos del modelo jerárquico de la persona
    personBody = new Model((char*)"Models/Persona/persona_cuerpo_sin_cabeza.obj");
    personHead = new Model((char*)"Models/Persona/persona_cabeza.obj");
    personRightArm = new Model((char*)"Models/Persona/persona_brazo_derecho.obj");
    personLeftArm = new Model((char*)"Models/Persona/persona_brazo_izquierdo.obj");
    personRightLeg = new Model((char*)"Models/Persona/persona_pierna_derecha.obj");
    personLeftLeg = new Model((char*)"Models/Persona/persona_pierna_izquierda.obj");
    jointMarker = new Model((char*)"Models/Persona/articulacion.obj");

    // Cargar los archivos del modelo jerárquico de la ardilla
    sqBody = new Model((char*)"Models/ardilla/cuerpo.obj");
    sqLeg1 = new Model((char*)"Models/ardilla/pata1.obj");
    sqLeg2 = new Model((char*)"Models/ardilla/pata2.obj");
    sqArm1 = new Model((char*)"Models/ardilla/mano1.obj");
    sqArm2 = new Model((char*)"Models/ardilla/mano2.obj");
    sqTail = new Model((char*)"Models/ardilla/cola.obj");



    // --- Inicialización de Animación Esquelética ---
    animShader = new Shader("Shader/anim.vs", "Shader/anim.frag");
    animacionPersonaje = new ModelAnim("Models/persona1/persona1.fbx");
    animacionPersonaje->initShaders(animShader->Program);

    // --- CARGAR TEXTURA DE IXANIK A LA FUERZA ---
    unsigned int texturaIxanik;
    glGenTextures(1, &texturaIxanik);
    glBindTexture(GL_TEXTURE_2D, texturaIxanik);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);

    // IMPORTANTE: Asegúrate de que esta ruta sea correcta
    unsigned char* data = stbi_load("Models/persona1/p3hero.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        printf("ERROR: No se encontro p3hero.png. Revisa la ruta.\n");
    }
    stbi_image_free(data);
    // ---------------------------------------------


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

    // Variables para controlar el recorrido del personaje
    glm::vec3 posPersona = glm::vec3(9.1f, 0.0f, -46.0f);
    int estadoPersona = 0;
    float timerPausa = 0.0f;     // <-- ¡Esta es la que faltaba!
    float anglePersona = 0.0f;

    // --- VARIABLES DEL CLON ---
    glm::vec3 posPersona2 = glm::vec3(10.0f, 0.0f, 22.0f); // X=5.0f para que camine a un lado
    int estadoPersona2 = 2;       // Inicia en 2 (Caminando de regreso)
    float anglePersona2 = 180.0f; // Nace mirando hacia el origen (-46)


    // Sincronizamos el reloj después de cargar los modelos para evitar el Pico de DeltaTime
    lastFrame = glfwGetTime();

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

        // --- LÓGICA DE RECORRIDO CONTINUO (Lobby FI) ---
        float velocidadPersona = 4.0f * deltaTime;

        if (estadoPersona == 0) { // Caminando hacia adelante
            posPersona.z += velocidadPersona;
            anglePersona = 0.0f;

            // CANDADO: Forzamos la posición para que no se pase ni un milímetro
            if (posPersona.z >= 22.0f) {
                posPersona.z = 22.0f;     // ¡Freno de mano!
                estadoPersona = 2;        // <-- CAMBIO: Pasa directo a caminar de regreso (Estado 2)
            }
        }
        else if (estadoPersona == 2) { // Caminando de regreso
            posPersona.z -= velocidadPersona;
            anglePersona = 180.0f;

            // CANDADO: Forzamos la posición en el origen
            if (posPersona.z <= -46.0f) {
                posPersona.z = -46.0f;  // ¡Freno de mano!
                estadoPersona = 0;        // <-- CAMBIO: Pasa directo a caminar de ida (Estado 0)
            }
        }

        // --- LÓGICA DE RECORRIDO DEL CLON (Inverso) ---
        if (estadoPersona2 == 0) { // Ida (De -46 a 22)
            posPersona2.z += velocidadPersona;
            anglePersona2 = 0.0f;

            if (posPersona2.z >= 22.0f) {
                posPersona2.z = 22.0f;
                estadoPersona2 = 2; // Pasa a regresar
            }
        }
        else if (estadoPersona2 == 2) { // Regreso (De 22 a -46)
            posPersona2.z -= velocidadPersona;
            anglePersona2 = 180.0f;

            if (posPersona2.z <= -46.0f) {
                posPersona2.z = -46.0f;
                estadoPersona2 = 0; // Pasa a avanzar
            }
        }

        //  verificar coordenadas
        printf("Posicion Z de Ixanik: %f\n", posPersona.z);

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();
        glUniform1i(glGetUniformLocation(shader.Program, "usarTexturaForzada"), 0);

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
// RENDERIZAR OBJETO CON CAMBIO DE TEXTURA
// ============================================================
        glm::mat4 modelObjeto = glm::mat4(1.0f);

        modelObjeto = glm::translate(modelObjeto, glm::vec3(11.7f, 2.0f, 18.1f));
        modelObjeto = glm::rotate(modelObjeto, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelObjeto = glm::scale(modelObjeto, glm::vec3(1.2f));

        glUniformMatrix4fv(
            glGetUniformLocation(shader.Program, "model"),
            1,
            GL_FALSE,
            glm::value_ptr(modelObjeto)
        );

        // Calcular qué textura se usa
        int texturaActual = ((int)(glfwGetTime() * 0.8f)) % NUM_TEXTURAS_OBJETO;

        // Activar textura forzada solo para la pantalla
        glUniform1i(glGetUniformLocation(shader.Program, "usarTexturaForzada"), 1);

        // Activar la textura animada
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texturasObjeto[texturaActual]);

        // Mandar la textura al shader
        glUniform1i(glGetUniformLocation(shader.Program, "texturaForzada"), 0);

        // Dibujar pantalla
        Pantalla.Draw(shader);

        // Apagar textura forzada
        glUniform1i(glGetUniformLocation(shader.Program, "usarTexturaForzada"), 0);
   

        //  ------- LETREROS -------
        // 
        // 

		glm::mat4 modelCartelesInfo = glm::mat4(1.0f);
		modelCartelesInfo = glm::translate(modelCartelesInfo, glm::vec3(11.7f, 2.3f, 14.0f)); // Posición del letrero
		//modelCartelesInfo = glm::rotate(modelCartelesInfo, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotar para que quede vertical
		//modelCartelesInfo = glm::scale(modelCartelesInfo, glm::vec3(1.0f)); // Ajustar el tamaño del letrero
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelCartelesInfo));
		CartelesInfo.Draw(shader);

		glm::mat4 modelAula = glm::mat4(1.0f);
		modelAula = glm::translate(modelAula, glm::vec3(11.75f, 2.2f, 0.0f)); // Posición del letrero
		modelAula = glm::rotate(modelAula, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
		modelAula = glm::rotate(modelAula, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAula));
        AulaMagna.Draw(shader);

		glm::mat4 modelFlecha = glm::mat4(1.0f);
		modelFlecha = glm::translate(modelFlecha, glm::vec3(11.75f, 3.3f, 13.5f)); // Posición del letrero
		modelFlecha = glm::rotate(modelFlecha, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
		modelFlecha = glm::rotate(modelFlecha, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelFlecha));
		PlacaFlecha.Draw(shader);

		glm::mat4 modelCartelAzul = glm::mat4(1.0f);
		modelCartelAzul = glm::translate(modelCartelAzul, glm::vec3(11.75f, 3.5f, 13.5f)); // Posición del letrero
		modelCartelAzul = glm::rotate(modelCartelAzul, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
		modelCartelAzul = glm::rotate(modelCartelAzul, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelCartelAzul));
		CartelAzul.Draw(shader);

		glm::mat4 modelUnionProf = glm::mat4(1.0f);
		modelUnionProf = glm::translate(modelUnionProf, glm::vec3(10.78f, 2.8f, 9.7f)); // Posición del letrero
		modelUnionProf = glm::rotate(modelUnionProf, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
		modelUnionProf = glm::rotate(modelUnionProf, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelUnionProf));
		UnionProf.Draw(shader);
        
        glm::mat4 modelUnionProfSA = glm::mat4(1.0f);
        modelUnionProfSA = glm::translate(modelUnionProfSA, glm::vec3(11.7f, 2.35f, 6.9f)); // Posición del letrero
        modelUnionProfSA = glm::rotate(modelUnionProfSA, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
        modelUnionProfSA = glm::rotate(modelUnionProfSA, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelUnionProfSA));
        UnionProfSA.Draw(shader);

		glm::mat4 modelConsejoTecnico = glm::mat4(1.0f);
		modelConsejoTecnico = glm::translate(modelConsejoTecnico, glm::vec3(11.7f, 2.35f, -4.5f)); // Posición del letrero
		modelConsejoTecnico = glm::rotate(modelConsejoTecnico, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
		modelConsejoTecnico = glm::rotate(modelConsejoTecnico, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelConsejoTecnico));
		ConsejoTecnico.Draw(shader);

        glm::mat4 modelSSA = glm::mat4(1.0f);
        modelSSA = glm::translate(modelSSA, glm::vec3(11.7f, 2.35f, -11.0f)); // Posición del letrero
        modelSSA = glm::rotate(modelSSA, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
        modelSSA = glm::rotate(modelSSA, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelSSA));
        SSA.Draw(shader);

        glm::mat4 modelDCSyH = glm::mat4(1.0f);
        modelDCSyH = glm::translate(modelDCSyH, glm::vec3(11.7f, 2.35f, -17.0f)); // Posición del letrero
        modelDCSyH = glm::rotate(modelDCSyH, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
        modelDCSyH = glm::rotate(modelDCSyH, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelDCSyH));
        DCSyH.Draw(shader);

        glm::mat4 modelFlecha2 = glm::mat4(1.0f);
        modelFlecha2 = glm::translate(modelFlecha2, glm::vec3(11.75f, 2.5f, 0.0f)); // Posición del letrero
        modelFlecha2 = glm::rotate(modelFlecha2, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
        modelFlecha2 = glm::rotate(modelFlecha2, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelFlecha2));
        PlacaFlecha.Draw(shader);

		glm::mat4 modelZonaRiesgo = glm::mat4(1.0f);
		modelZonaRiesgo = glm::translate(modelZonaRiesgo, glm::vec3(10.8f, 3.0f, -22.8f)); // Posición del letrero
		modelZonaRiesgo = glm::scale(modelZonaRiesgo, glm::vec3(1.5f));
		modelZonaRiesgo = glm::rotate(modelZonaRiesgo, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotar para que quede vertical
		modelZonaRiesgo = glm::rotate(modelZonaRiesgo, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotar para que quede orientado hacia el pasillo
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelZonaRiesgo));
        ZonaRiesgo.Draw(shader);


		glm::mat4 modelLona = glm::mat4(1.0f);
		modelLona = glm::translate(modelLona, glm::vec3(10.8f, 1.0f, -18.0f)); // Posición del letrero
		modelLona = glm::rotate(modelLona, glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotar para que quede orientado hacia el pasillo
		modelLona = glm::scale(modelLona, glm::vec3(2.0f)); // Escalar para que se vea mejor, ya que el modelo original es muy pequeño
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelLona));
		Lona.Draw(shader);




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
        // RENDERIZAR PERSONA 
        // ============================================================
        if (personVisible && personBody != nullptr && personHead != nullptr && personRightArm != nullptr
            && personLeftArm != nullptr && personRightLeg != nullptr && personLeftLeg != nullptr) {

            glm::mat4 personBase = glm::mat4(1.0f);
            personBase = glm::translate(personBase, personPos + glm::vec3(0.0f, personBodyBob, 0.0f));
            personBase = glm::rotate(personBase, glm::radians(personYaw), glm::vec3(0.0f, 1.0f, 0.0f));
            personBase = glm::rotate(personBase, glm::radians(personBodyForwardLean), glm::vec3(1.0f, 0.0f, 0.0f));
            personBase = glm::rotate(personBase, glm::radians(personBodySideLean), glm::vec3(0.0f, 0.0f, 1.0f));
            personBase = glm::scale(personBase, personScale);

            //Cuerpo sin cabeza
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(personBase));
            personBody->Draw(shader);

            //Cabeza
            glm::vec3 pivotHead = glm::vec3(0.02f, 1.56f, 0.06f);
            glm::mat4 headMat = personBase;
            headMat = glm::translate(headMat, pivotHead);
            headMat = glm::rotate(headMat, glm::radians(personHeadYaw), glm::vec3(0.0f, 1.0f, 0.0f));
            headMat = glm::rotate(headMat, glm::radians(personHeadPitch), glm::vec3(1.0f, 0.0f, 0.0f));
            headMat = glm::translate(headMat, -pivotHead);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(headMat));
            personHead->Draw(shader);

            //Brazo derecho
            glm::vec3 pivotRightArm = glm::vec3(0.13f, 1.45f, 0.08f);
            glm::mat4 rightArmMat = personBase;
            rightArmMat = glm::translate(rightArmMat, pivotRightArm);
            rightArmMat = glm::rotate(rightArmMat, glm::radians(personRightArmX), glm::vec3(1.0f, 0.0f, 0.0f));
            rightArmMat = glm::rotate(rightArmMat, glm::radians(personRightArmY), glm::vec3(0.0f, 1.0f, 0.0f));
            rightArmMat = glm::rotate(rightArmMat, glm::radians(personRightArmZ), glm::vec3(0.0f, 0.0f, 1.0f));
            rightArmMat = glm::translate(rightArmMat, -pivotRightArm);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(rightArmMat));
            personRightArm->Draw(shader);

            //Brazo izquierdo
            glm::vec3 pivotLeftArm = glm::vec3(-0.13f, 1.45f, 0.08f);
            glm::mat4 leftArmMat = personBase;
            leftArmMat = glm::translate(leftArmMat, pivotLeftArm);
            leftArmMat = glm::rotate(leftArmMat, glm::radians(personLeftArmX), glm::vec3(1.0f, 0.0f, 0.0f));
            leftArmMat = glm::rotate(leftArmMat, glm::radians(personLeftArmY), glm::vec3(0.0f, 1.0f, 0.0f));
            leftArmMat = glm::rotate(leftArmMat, glm::radians(personLeftArmZ), glm::vec3(0.0f, 0.0f, 1.0f));
            leftArmMat = glm::translate(leftArmMat, -pivotLeftArm);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(leftArmMat));
            personLeftArm->Draw(shader);

            //Pie derecho
            glm::vec3 pivotRightLeg = glm::vec3(0.08f, 0.92f, 0.03f);
            glm::mat4 rightLegMat = personBase;
            rightLegMat = glm::translate(rightLegMat, pivotRightLeg);
            rightLegMat = glm::rotate(rightLegMat, glm::radians(personRightLegX), glm::vec3(1.0f, 0.0f, 0.0f));
            rightLegMat = glm::translate(rightLegMat, -pivotRightLeg);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(rightLegMat));
            personRightLeg->Draw(shader);

            //Pie izquierdo
            glm::vec3 pivotLeftLeg = glm::vec3(-0.08f, 0.92f, 0.03f);
            glm::mat4 leftLegMat = personBase;
            leftLegMat = glm::translate(leftLegMat, pivotLeftLeg);
            leftLegMat = glm::rotate(leftLegMat, glm::radians(personLeftLegX), glm::vec3(1.0f, 0.0f, 0.0f));
            leftLegMat = glm::translate(leftLegMat, -pivotLeftLeg);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(leftLegMat));
            personLeftLeg->Draw(shader);


            // ============================================================
            // Aritciculaciones 
            // ============================================================
            if (showPersonJoints && jointMarker != nullptr) {

                auto DrawJoint = [&](glm::vec3 pivot, float size) {
                    glm::mat4 jointMat = personBase;
                    //jointMat = glm::translate(jointMat, pivot);
                    jointMat = glm::translate(jointMat, pivot + glm::vec3(0.0f, 0.045f, 0.0f));
                    jointMat = glm::scale(jointMat, glm::vec3(size));

                    glUniformMatrix4fv(
                        glGetUniformLocation(shader.Program, "model"),
                        1,
                        GL_FALSE,
                        glm::value_ptr(jointMat)
                    );

                    jointMarker->Draw(shader);
                    };

                // Cuello / cabeza
                DrawJoint(pivotHead, 0.06f);
                // Hombros
                DrawJoint(pivotRightArm, 0.37f);
                DrawJoint(pivotLeftArm, 0.37f);
                // Caderas / piernas
                DrawJoint(pivotRightLeg, 0.06f);
                DrawJoint(pivotLeftLeg, 0.06f);
            }
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
            sqBase = glm::scale(sqBase, glm::vec3(0.2f));

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

        // RENDERIZAR PERSONA CON ESQUELETO (PASO 4)

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
        glUniform3f(glGetUniformLocation(animShader->Program, "viewPos"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // 3. Dibujamos
        glm::mat4 modelAnim = glm::mat4(1.0f);
        modelAnim = glm::translate(modelAnim, posPersona);
        modelAnim = glm::rotate(modelAnim, glm::radians(anglePersona), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim = glm::scale(modelAnim, glm::vec3(0.013f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAnim));
        printf("Posicion Z de Ixanik: %f\n", posPersona.z);

        // 1. Prendemos el bypass
        glUniform1i(glGetUniformLocation(animShader->Program, "usarForzada"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texturaIxanik);
        glUniform1i(glGetUniformLocation(animShader->Program, "texturaForzada"), 1);

        // --- DIBUJAMOS A LA PERSONA 1  ---
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAnim));
        animacionPersonaje->Draw(*animShader);

        // --- DIBUJAMOS A LA PERSONA 2 (El Clon) ---
        glm::mat4 modelAnim2 = glm::mat4(1.0f);
        modelAnim2 = glm::translate(modelAnim2, posPersona2);
        modelAnim2 = glm::rotate(modelAnim2, glm::radians(anglePersona2), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnim2 = glm::scale(modelAnim2, glm::vec3(0.013f)); // La misma escala pequeñita

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAnim2));
        animacionPersonaje->Draw(*animShader); 

        // 4. Apagamos el bypass 
        glUniform1i(glGetUniformLocation(animShader->Program, "usarForzada"), 0);

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

    delete personBody;
    delete personHead;
    delete personRightArm;
    delete personLeftArm;
    delete personRightLeg;
    delete personLeftLeg;

    delete sqBody;
    delete sqLeg1;
    delete sqLeg2;
    delete sqArm1;
    delete sqArm2;
    delete sqTail;

    delete animacionPersonaje;
    delete animShader;

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