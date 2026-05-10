#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in ivec4 bone_ids;
layout (location = 6) in vec4 weights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 300;
uniform mat4 bones[MAX_BONES];

void main()
{
    // 1. Forzar a que los pesos sumen exactamente 1.0
    float totalW = weights[0] + weights[1] + weights[2] + weights[3];
    vec4 w = weights;
    if (totalW > 0.0) {
        w = w / totalW; 
    }

    // 2. Calcular la posición con los huesos
    mat4 bone_transform = bones[bone_ids[0]] * w[0];
    bone_transform += bones[bone_ids[1]] * w[1];
    bone_transform += bones[bone_ids[2]] * w[2];
    bone_transform += bones[bone_ids[3]] * w[3];

    // 3. Seguro anti-spikes (si no hay hueso, forzar posición original)
    if (bone_transform[0][0] == 0.0 && bone_transform[1][1] == 0.0) {
        bone_transform = mat4(1.0);
    }

    // 4. Aplicar deformación al vértice
    vec4 boned_position = bone_transform * vec4(aPos, 1.0);

    // 5. Calcular posiciones finales para la pantalla
    FragPos = vec3(model * boned_position);
    Normal = mat3(model * bone_transform) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoords = aTexCoords;
}