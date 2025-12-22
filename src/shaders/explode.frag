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

    vec3 color = texture(texture_diffuse1, fin.uv).rgb;

    FragColor = vec4(color, 1.0);
}
