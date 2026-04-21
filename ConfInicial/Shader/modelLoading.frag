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

// Fill light (rebote ambiental, simula HDRI suave)
uniform vec3 fillLightDir;
uniform vec3 fillLightColor;

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

    // --- Luz principal ---

    // Ambient
    vec3 ambient = lightAmbient * baseColor.rgb;

    // Diffuse principal
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = lightDiffuse * diff * baseColor.rgb;

    // Specular limitado (evita quemado blanco)
    vec3 reflectDir = reflect(-lightDirection, norm);
    float shininess = max(material_shininess, 1.0);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);
    spec = min(spec, 0.4);  // clamp especular para no quemar
    vec3 specular = lightSpecular * spec * material_specular;

    // --- Fill light (rebote desde abajo/costado, ilumina sombras) ---
    vec3 fillDir = normalize(fillLightDir);
    float fillDiff = max(dot(norm, fillDir), 0.0) * 0.6;
    vec3 fill = fillLightColor * fillDiff * baseColor.rgb;

    vec3 finalRGB = ambient + diffuse + specular + fill;
    float finalAlpha = baseColor.a * material_alpha;

    // --- Vidrio con efecto Fresnel ---
    if (material_alpha < 0.99)
    {
        // Fresnel: mas reflejo en angulos rasantes
        float fresnel = pow(1.0 - max(dot(norm, viewDirection), 0.0), 3.0);
        fresnel = clamp(fresnel, 0.0, 1.0);

        // Tinte azul-gris del vidrio
        vec3 glassTint = vec3(0.75, 0.85, 0.90);

        // Reflejo especular fuerte en vidrio
        float glassSpec = pow(max(dot(viewDirection, reflectDir), 0.0), 64.0);
        vec3 glassReflect = vec3(glassSpec * 0.6);

        // Combinar: color base translucido + tinte + fresnel + reflejo
        finalRGB = mix(finalRGB * 0.5, glassTint, 0.25) + glassReflect + fresnel * 0.15;
        finalAlpha = mix(material_alpha, 0.75, fresnel);  // mas opaco en angulos rasantes
    }

    color = vec4(clamp(finalRGB, 0.0, 1.0), clamp(finalAlpha, 0.0, 1.0));
}
