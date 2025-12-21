#version 330 core

// TODO: Implement glass shading with schlick method

in vec3 FragPos;
in vec3 Normal;
in vec3 I; // 入射/視線方向

uniform float AIR_coeff;      // n1 = 1.0
uniform float GLASS_coeff;    // n2 = 1.52

uniform samplerCube skybox;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(Normal);

    // ===== Reflection =====
    vec3 R_dir = reflect(-I, N);
    vec3 C_reflect = texture(skybox, R_dir).rgb;

    // ===== Refraction =====
    float eta = AIR_coeff / GLASS_coeff; // 折射率比
    vec3 T_dir = refract(-I, N, eta); // 折射

    // 如果 T_dir = 0 → 全反射（refract() 會自動變 0）
    vec3 C_refract;
    if (length(T_dir) == 0.0) {
        // 如果發生全內反射，則只有反射
        C_refract = C_reflect; 
    } else {
        C_refract = texture(skybox, T_dir).rgb;
    }

    // ===== Schlick Approximation =====
    float R0 = pow((AIR_coeff - GLASS_coeff) / (AIR_coeff + GLASS_coeff), 2.0);

    float cosTheta = clamp(dot(I, N), -1.0, 1.0);

    float R_theta = R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5.0);

    vec3 C_final = R_theta * C_reflect + (1.0 - R_theta) * C_refract;

    FragColor = vec4(C_final, 1.0);
}