#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D texture_diffuse1;
uniform int hasDiffuseTexture;
uniform vec3 material_diffuse;
uniform float material_alpha;

void main()
{
    vec4 baseColor;

    if (hasDiffuseTexture == 1)
        baseColor = texture(texture_diffuse1, TexCoords);
    else
        baseColor = vec4(material_diffuse, 1.0);

    vec3 finalRGB = baseColor.rgb;
    float finalAlpha = 1.0;

    if (material_alpha < 1.0)
    {
        vec3 glassTint = vec3(0.78, 0.84, 0.88);
        finalRGB = mix(baseColor.rgb, glassTint, 0.35);
        finalAlpha = 0.55;
    }

    color = vec4(finalRGB, finalAlpha);
}