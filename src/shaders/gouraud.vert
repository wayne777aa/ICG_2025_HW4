#version 330 core

// TODO:
// Implement Gouraud shading

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float matGloss;

out vec3 vColor;       // 已經算好的光照直接傳到 fragment shader
out vec2 vTexCoord;

void main()
{
    vec3 FragPos = vec3(model * vec4(inPos, 1.0));
    vec3 Normal = normalize(mat3(transpose(inverse(model))) * inNormal);

    vec3 viewPos = vec3(inverse(view)[3]);   // camera position
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir  = normalize(viewPos - FragPos);

    // ===== Ambient =====
    vec3 ambient = lightAmbient * matAmbient;

    // ===== Diffuse =====
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = lightDiffuse * matDiffuse * diff;

    // ===== Specular (Phong) =====
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), matGloss);
    vec3 specular = lightSpecular * matSpecular * spec;

    // Final vertex lighting color
    vColor = ambient + diffuse + specular;
    vTexCoord = inTexCoord;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
