#version 330 core

// TODO: Implement bling-phong shading
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;

// Light
uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

// Material
uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float matGloss;

uniform sampler2D texture_diffuse1;

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);

    // ambient
    vec3 ambient = lightAmbient * matAmbient;

    // diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightDiffuse * matDiffuse;

    // Blinn-Phong specular
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), matGloss);
    vec3 specular = spec * lightSpecular * matSpecular;

    vec3 tex = texture(texture_diffuse1, TexCoord).rgb;
    vec3 result = (ambient + diffuse + specular) * tex;

    FragColor = vec4(result, 1.0);	
}