#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

uniform mat4 view;
uniform mat4 projection;
uniform float uAmp; // 爆炸幅度：外圈->0

in VS_OUT { vec3 wPos; } gin[];
out vec3 gBary;

float hash1(float x){ return fract(sin(x * 12.9898) * 43758.5453); }
vec3 hashDir(int id){
    float a = hash1(float(id) + 1.0) * 6.2831853;
    float z = hash1(float(id) + 2.0) * 2.0 - 1.0;
    float r = sqrt(max(0.0, 1.0 - z*z));
    return vec3(r*cos(a), r*sin(a), z);
}

void main(){
    vec3 e1 = gin[1].wPos - gin[0].wPos;
    vec3 e2 = gin[2].wPos - gin[0].wPos;
    vec3 faceN = normalize(cross(e1, e2));
    vec3 rnd   = hashDir(gl_PrimitiveID);
    vec3 dir   = normalize(mix(faceN, rnd, 0.35)); // 這個 mix 係數要跟 explode.geom 一致

    vec3 bary[3] = vec3[3](vec3(1,0,0), vec3(0,1,0), vec3(0,0,1));

    for(int i=0;i<3;i++){
        gBary = bary[i];
        vec3 p = gin[i].wPos + dir * uAmp;
        gl_Position = projection * view * vec4(p, 1.0);
        gl_Position.z -= 1e-4 * gl_Position.w;
        EmitVertex();
    }
    EndPrimitive();
}