#version 330 core

// TODO: Implement CubeMap shading
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat4 viewNoTrans = view; // `view` is already the rotation part from main.cpp
    TexCoords = aPos;
    vec4 pos = projection * viewNoTrans * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  