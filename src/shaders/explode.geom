#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 view;
uniform mat4 projection;

uniform float uTime;      // 距離觸發爆炸的時間（秒）
// uniform float uDuration;  // 爆炸持續時間
uniform float uBlastTime;  // 爆炸出去持續時間
uniform float uHoldTime;    // 停在最大位移多久（秒）
uniform float uReturnTime;  // 爆炸返回持續時間
uniform float uStrength;  // 炸開距離尺度

in VS_OUT {
    vec3 wPos;
    vec3 wN;
    vec2 uv;
} gin[];

out GS_OUT {
    vec3 wPos;
    vec3 wN;
    vec2 uv;
    float t; // 0~1 用於 fragment 做 fade / 發光
} gout;

// 很粗糙但夠用的 hash（別期待「真隨機」）
float hash1(float x){
    return fract(sin(x * 12.9898) * 43758.5453);
}

vec3 hashDir(int id){
    float a = hash1(float(id) + 1.0) * 6.2831853;
    float z = hash1(float(id) + 2.0) * 2.0 - 1.0;
    float r = sqrt(max(0.0, 1.0 - z*z));
    return vec3(r*cos(a), r*sin(a), z);
}

void main(){
    float t = max(uTime, 0.0);
    float amp = 0.0;

    float t0 = uBlastTime;
    float t1 = uBlastTime + uHoldTime;
    float t2 = uBlastTime + uHoldTime + uReturnTime;   
    float total = t2;                 // uBlast + uHold + uReturn
    float t01 = clamp(t / total, 0.0, 1.0); 
    if (t < t0) {
        float x = t / uBlastTime;
        // 爆出去：前快後慢（easeOut）
        float e = 1.0 - pow(1.0 - x, 4.0);
        amp = uStrength * e;
    } else if (t < t1) {
        // 停頓：直接固定在最大
        amp = uStrength;
    } else if (t < t2) {
        float x = (t - t1) / uReturnTime;
        x = clamp(x, 0.0, 1.0);
        // 回來：慢慢回到 0（easeOut 讓尾端更慢）
        float e = 1.0 - pow(1.0 - x, 3.0);
        amp = uStrength * (1.0 - e);
    } else {
        amp = 0.0; // 結束後回到原位
    }

    // face normal（用 world pos 算）
    vec3 e1 = gin[1].wPos - gin[0].wPos;
    vec3 e2 = gin[2].wPos - gin[0].wPos;
    vec3 faceN = normalize(cross(e1, e2));

    // 每個 primitive 給一個固定的隨機方向（避免整片同方向）
    vec3 rnd = hashDir(gl_PrimitiveID);

    // 最終位移方向：以 face normal 為主，混一點亂數
    vec3 dir = normalize(mix(faceN, rnd, 0.35));

    for(int i = 0; i < 3; i++){
        vec3 p = gin[i].wPos + dir * amp;

        gout.wPos = p;
        gout.wN   = gin[i].wN;
        gout.uv   = gin[i].uv;
        gout.t  = t01;

        gl_Position = projection * view * vec4(p, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}
