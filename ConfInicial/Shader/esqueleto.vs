#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec4 boneIds; 
layout (location = 6) in vec4 weights;

uniform mat4 finalBonesMatrices[100]; // Arreglo de huesos
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vec4 totalPosition = vec4(0.0);
    for(int i = 0 ; i < 4 ; i++) {
        if(boneIds[i] == -1) continue;
        if(boneIds[i] >= 100) {
            totalPosition = vec4(aPos, 1.0);
            break;
        }
        // Calculamos la posición del vértice multiplicada por la matriz del hueso y su peso
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0);
        totalPosition += localPosition * weights[i];
    }
    gl_Position = projection * view * model * totalPosition;
}