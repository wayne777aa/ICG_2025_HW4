#version 330 core

// TODO:
// Implement Gouraud shading

in vec3 vColor;
in vec2 vTexCoord;

uniform sampler2D texture_diffuse1;

out vec4 FragColor;

void main()
{
    vec3 texColor = texture(texture_diffuse1, vTexCoord).rgb;
    FragColor = vec4(vColor * texColor, 1.0);
}