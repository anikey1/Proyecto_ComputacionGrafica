//Proyecto Final - IXANIK
//Anikey Andrea Gomez Guzman
//319323290

#include <string>
#include <iostream>
#include <vector>

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

Camera camera(glm::vec3(8.0f, 2.2, 18.0f));
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

//// --- Animacion A2 ---
//std::vector<Model*> animFrames;
//int   animCurrentFrame = 0;
//float animTimer = 0.0f;
//float animFrameDuration = 0.033f; // ~30fps

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "IXANIK - Lobby FI", nullptr, nullptr);
    if (nullptr == window)
    {
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
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // --- Carga stands ---
    for (int i = 0; i < (int)standConfigs.size(); i++)
        stands.push_back(new Model((char*)standConfigs[i].path.c_str()));

    // --- Carga animacion A2 (250 frames) ---
   // p40 es un solo frame aparte, los de animacion van de p410001 a p410250
   /* for (int i = 1; i <= 10; i++)
    {
        char path[256];
        sprintf_s(path, "Animaciones/A2/p%d.obj", i);
        animFrames.push_back(new Model(path));
    }*/

    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.01f, 1000.0f
    );

    Model lobby((char*)"Models/ModeloLobby/final.obj");

    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

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
        for (int i = 0; i < (int)stands.size(); i++)
        {
            if (!standConfigs[i].visible) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, standConfigs[i].pos);
            model = glm::rotate(model, glm::radians(standConfigs[i].rotY), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, standConfigs[i].scale);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            stands[i]->Draw(shader);
        }

        //// --- Animacion A2: personaje junto al stand ---
        //animTimer += deltaTime;
        //if (animTimer >= animFrameDuration)
        //{
        //    animTimer = 0.0f;
        //    animCurrentFrame = (animCurrentFrame + 1) % 10;
        //}

        glm::mat4 modelPersonaje = glm::mat4(1.0f);
        modelPersonaje = glm::translate(modelPersonaje, glm::vec3(5.7f, 0.0f, 9.2f)); // junto al s3, ajusta
        modelPersonaje = glm::rotate(modelPersonaje, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelPersonaje = glm::scale(modelPersonaje, glm::vec3(1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPersonaje));
        /*animFrames[animCurrentFrame]->Draw(shader);*/

        glfwSwapBuffers(window);
    }

    // Limpieza
    for (Model* s : stands) delete s;
    stands.clear();
    /*for (Model* f : animFrames) delete f;
    animFrames.clear();*/

    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)   keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_1) standConfigs[0].visible = !standConfigs[0].visible;
        if (key == GLFW_KEY_2) standConfigs[1].visible = !standConfigs[1].visible;
        if (key == GLFW_KEY_3) standConfigs[2].visible = !standConfigs[2].visible;
        if (key == GLFW_KEY_4) standConfigs[3].visible = !standConfigs[3].visible;
        if (key == GLFW_KEY_5) standConfigs[4].visible = !standConfigs[4].visible;
        if (key == GLFW_KEY_6) standConfigs[5].visible = !standConfigs[5].visible;
        if (key == GLFW_KEY_7) standConfigs[6].visible = !standConfigs[6].visible;
        if (key == GLFW_KEY_8) standConfigs[7].visible = !standConfigs[7].visible;
        if (key == GLFW_KEY_0)
            for (auto& s : standConfigs) s.visible = true;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) { lastX = xPos; lastY = yPos; firstMouse = false; }
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    lastX = xPos; lastY = yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}