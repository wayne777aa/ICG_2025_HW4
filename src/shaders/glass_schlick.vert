#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// TODO: Implement glass shading with schlick method

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

out vec3 FragPos;
out vec3 Normal;
out vec3 I; // 入射/視線方向

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);

    I = normalize(viewPos - FragPos);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}