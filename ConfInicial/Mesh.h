#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"

using namespace std;

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture
{
    GLuint id;
    string type;
    aiString path;
};

class Mesh
{
public:
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    glm::vec3 diffuseColor; // color base Kd cuando no hay textura
    float alpha;            // opacidad del material

    Mesh(vector<Vertex> vertices,
        vector<GLuint> indices,
        vector<Texture> textures,
        glm::vec3 diffuseColor = glm::vec3(0.6f),
        float alpha = 1.0f)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->diffuseColor = diffuseColor;
        this->alpha = alpha;
        this->setupMesh();
    }

    void Draw(Shader shader)
    {
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;

        bool hasDiffuse = false;

        for (GLuint i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);

            stringstream ss;
            string number;
            string name = this->textures[i].type;

            if (name == "texture_diffuse")
            {
                ss << diffuseNr++;
                hasDiffuse = true;
            }
            else if (name == "texture_specular")
            {
                ss << specularNr++;
            }

            number = ss.str();
            glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
        }

        // Pasar al shader si tiene textura difusa
        glUniform1i(glGetUniformLocation(shader.Program, "hasDiffuseTexture"), hasDiffuse ? 1 : 0);

        // Pasar color difuso base
        glUniform3f(glGetUniformLocation(shader.Program, "material_diffuse"),
            diffuseColor.r, diffuseColor.g, diffuseColor.b);

        // Pasar alpha del material
        glUniform1f(glGetUniformLocation(shader.Program, "material_alpha"), alpha);

        // Shininess
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);

        if (alpha < 1.0f)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
        }
        else
        {
            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
        }

        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        for (GLuint i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

private:
    GLuint VAO, VBO, EBO;

    void setupMesh()
    {
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindVertexArray(this->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};