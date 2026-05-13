# IXANIK - Lobby FI

IXANIK es un entorno virtual interactivo desarrollado en C++ y OpenGL que recrea el lobby del auditorio de la Facultad de Ingeniería. El proyecto permite recorrer un espacio tridimensional ambientado para eventos universitarios, con stands distribuidos dentro del espacio, señalética, personajes animados, fauna, pantallas informativas y elementos dinámicos que enriquecen la experiencia visual.

El sistema integra técnicas de computación gráfica como modelado jerárquico, carga de modelos externos, texturizado, iluminación, animación por keyframes, animación esquelética y navegación en tiempo real.

## Integrantes

- 319323290
- 320260366
- 320110450

## Descripción del proyecto

El proyecto representa un lobby universitario preparado para la colocación de stands durante eventos académicos. La escena permite visualizar la distribución de estos elementos dentro de un espacio virtual, además de incorporar componentes animados que simulan actividad dentro del entorno.

El escenario incluye:

- Lobby principal modelado en 3D.
- Stands distribuidos dentro del espacio.
- Letreros y señalética institucional.
- Pantalla dinámica con cambio de contenido.
- Personajes animados.
- Animales animados mediante transformaciones jerárquicas.
- Navegación libre mediante cámara en primera persona.
- Interacción básica para seleccionar, mover, rotar, escalar y ocultar stands.

## Tecnologías utilizadas

El proyecto fue desarrollado con una arquitectura basada en OpenGL moderno y C++.

### Lenguaje

- C++

### Bibliotecas y herramientas

- OpenGL 3.3 Core Profile
- GLFW
- GLEW
- GLM
- SOIL2
- stb_image
- Assimp
- GLSL
- Visual Studio

## Estructura del repositorio

```text
Proyecto_ComputacionGrafica/
│
├── ConInicial/
│   ├── Models/
│   ├── SOIL2/
│   ├── Shader/
│   ├── Camera.h
│   ├── ConfInicial.vcxproj
│   ├── ConfInicial.vcxproj.filters
│   ├── Mesh.h
│   ├── Model.h
│   ├── Shader.h
│   ├── main.cpp
│   ├── meshAnim.h
│   ├── modelAnim.h
│   ├── stb_image.h
│   ├── assimp-vc140-mt.dll
│   └── glew32.dll
│
├── External Libraries/
│   ├── GLEW/
│   ├── GLFW/
│   ├── SOIL2/lib/
│   ├── assimp/
│   └── glm/glm/
│
├── .gitattributes
├── .gitignore
├── ConfInicial.sln
├── CppProperties.json
└── README.md