#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 boneIds; 
layout (location = 4) in vec4 weights;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 localNormal = vec3(0.0f);
    
    bool hasBones = false;
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] >= 0 && boneIds[i] < MAX_BONES)
        {
            hasBones = true;
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            totalPosition += localPosition * weights[i];
            mat3 boneMat = mat3(finalBonesMatrices[boneIds[i]]);
            localNormal += boneMat * aNormal * weights[i];
        }
    }
    
    // Si la malla no tiene huesos (seguridad), se dibuja normal
    if(!hasBones) {
        totalPosition = vec4(aPos, 1.0f);
        localNormal = aNormal;
    }

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * localNormal);
    FragPos = vec3(model * totalPosition);
    TexCoords = aTexCoords;
    
    gl_Position = projection * view * model * totalPosition;
}