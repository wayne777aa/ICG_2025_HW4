#version 330 core

// TODO: Implement metallic shading

in vec3 FragPos;
in vec3 Normal;
in vec3 I;

uniform vec3 lightPos;
uniform vec3 lightDiffuse;

uniform vec3 bias;
uniform float alpha;

uniform samplerCube skybox;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(Normal);

    // ===== compute light direction =====
    vec3 lightDir = normalize(lightPos - FragPos);

    // ===== Lambert diffuse + bias =====
    vec3 B = max(dot(N, lightDir), 0.0) * lightDiffuse + bias;

    // ===== reflection direction =====
    vec3 reflectDir = reflect(-I, N);

    // ===== sample environment =====
    vec3 C_reflect = texture(skybox, reflectDir).rgb;

    // ===== final = α * diffuse + (1 - α) * reflect =====
    vec3 finalColor = alpha * B + (1.0 - alpha) * C_reflect;

    FragColor = vec4(finalColor, 1.0);
}