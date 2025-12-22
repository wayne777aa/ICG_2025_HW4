#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec3 wPos;
    vec3 wN;
    vec2 uv;
} vout;

void main(){
    vec4 wp = model * vec4(aPos, 1.0);
    vout.wPos = wp.xyz;
    vout.wN   = normalize(mat3(transpose(inverse(model))) * aNormal);
    vout.uv   = aTexCoord;

    // 先不投影，留給 GS 做位移後再投影也行；
    // 但這裡先給一個值避免某些 driver 抱怨
    gl_Position = projection * view * wp;
}
