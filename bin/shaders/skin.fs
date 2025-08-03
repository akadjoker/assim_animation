#version 330 core

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D diffuseTexture;

uniform vec3 lightDir;    // Direção da luz (normalizada, em world space)
uniform vec3 lightColor;  // Cor da luz (ex: vec3(1.0, 1.0, 1.0))
uniform vec3 ambientColor; // Cor do ambiente (ex: vec3(0.2, 0.2, 0.2))

void main()
{
    vec3 texColor = texture(diffuseTexture, TexCoords).rgb;

    // Normalize normal para segurança
    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);  // Luz é direcional -> inverso da direção

    // Lambertian diffuse
    float diff = max(dot(N, L), 0.0);

    vec3 diffuse = diff * lightColor;
    vec3 ambient = ambientColor;

    vec3 finalColor = (ambient + diffuse) * texColor;

    FragColor = vec4(finalColor, 1.0);
}
