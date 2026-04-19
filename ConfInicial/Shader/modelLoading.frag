#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
out vec4 color;

uniform sampler2D texture_diffuse1;
uniform int hasDiffuseTexture;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;
uniform float material_alpha;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

void main()
{
    vec4 baseColor;
    if (hasDiffuseTexture == 1)
        baseColor = texture(texture_diffuse1, TexCoords);
    else
        baseColor = vec4(material_diffuse, 1.0);

    if (baseColor.a < 0.05)
        discard;

    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir);
    vec3 viewDirection = normalize(viewPos - FragPos);

    // Ambient
    vec3 ambient = lightAmbient * baseColor.rgb;

    // Diffuse
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = lightDiffuse * diff * baseColor.rgb;

    // Specular Phong
    vec3 reflectDir = reflect(-lightDirection, norm);
    float shininess = max(material_shininess, 1.0);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);
    vec3 specular = lightSpecular * spec * material_specular;

    vec3 finalRGB = ambient + diffuse + specular;
    float finalAlpha = baseColor.a;

    // Vidrio
    if (material_alpha < 0.99)
    {
        vec3 glassTint = vec3(0.82, 0.88, 0.92);
        finalRGB = mix(finalRGB, glassTint, 0.35);
        finalAlpha = material_alpha;
    }

    color = vec4(clamp(finalRGB, 0.0, 1.0), finalAlpha);
}
