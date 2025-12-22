#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

uniform mat4 view;
uniform mat4 projection;

in VS_OUT { vec3 wPos; } gin[];

out vec3 gBary;

void main(){
    vec3 bary[3] = vec3[3](vec3(1,0,0), vec3(0,1,0), vec3(0,0,1));
    for(int i=0;i<3;i++){
        gBary = bary[i];
        gl_Position = projection * view * vec4(gin[i].wPos, 1.0);
        gl_Position.z -= 1e-4 * gl_Position.w; // 偏移一點點 z 值，避免 z-fighting
        EmitVertex();
    }
    EndPrimitive();
}
