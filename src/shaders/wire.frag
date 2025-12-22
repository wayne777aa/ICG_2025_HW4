#version 330 core
out vec4 FragColor;

in vec3 gBary;
uniform vec3  uLineColor;
uniform float uLineWidth; // 建議 1~3

void main(){
    float d = min(min(gBary.x, gBary.y), gBary.z);
    float w = fwidth(d) * uLineWidth;
    float line = 1.0 - smoothstep(0.0, w, d);
    FragColor = vec4(uLineColor, line); // alpha = 線強度
}
