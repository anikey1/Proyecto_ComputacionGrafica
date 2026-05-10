#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;
in vec2 TexCoords;

struct Material {
    vec3 specular; 
    float shininess;
};
struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// --- EL INTERRUPTOR PARA EL BYPASS ---
uniform sampler2D texture_diffuse1; // La ruta rota del FBX
uniform sampler2D texturaForzada;   // Tu p3hero.png
uniform bool usarForzada;           // El switch
// -------------------------------------

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    // 1. Aquí decidimos qué textura usar
    vec4 colorTextura;
    if(usarForzada) {
        colorTextura = texture(texturaForzada, TexCoords); // Forzamos p3hero.png
    } else {
        colorTextura = texture(texture_diffuse1, TexCoords); // Usamos lo del FBX
    }

    // ambient
    vec3 ambient  = light.ambient * vec3(colorTextura);
    
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = light.diffuse * diff * vec3(colorTextura);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular); 
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}