#version 330 core
out vec4 FragColor;

in GS_OUT {
    vec3 wPos;
    vec3 wN;
    vec2 uv;
    float t;
} fin;

uniform vec3 lightPos;
uniform vec3 lightDiffuse;

uniform vec3 viewPos;

uniform sampler2D texture_diffuse1;

void main(){
    vec3 N = normalize(fin.wN);
    vec3 L = normalize(lightPos - fin.wPos);
    vec3 V = normalize(viewPos - fin.wPos);

    float diff = max(dot(N, L), 0.0);

    // 爆炸中段更亮
    float glow = smoothstep(0.05, 0.35, fin.t) * (1.0 - smoothstep(0.55, 1.0, fin.t));

    vec3 albedo = texture(texture_diffuse1, fin.uv).rgb;
    vec3 color = albedo * (0.15 + diff) + glow * vec3(0.3);
    // vec3 color = albedo * (0.15 + diff);

    // 後段淡出
    float alpha = 1.0 - smoothstep(0.7, 1.0, fin.t);
    FragColor = vec4(color, alpha);
}
