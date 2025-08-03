#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 5) in ivec4 aBones;
layout(location = 6) in vec4 aWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 180;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 Joints[MAX_BONES];

void main() 
{
    vec4 totalPosition = vec4(0.0);
    vec4 totalNormal = vec4(0.0);

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (aBones[i] == -1 || aWeights[i] == 0.0) continue;

        mat4 jointTransform = Joints[aBones[i]];

        totalPosition += jointTransform * vec4(aPos, 1.0) * aWeights[i];
        totalNormal   += jointTransform * vec4(aNormal, 0.0) * aWeights[i];
    }

    Normal = mat3(transpose(inverse(model))) * totalNormal.xyz;
    FragPos = vec3(model * totalPosition);

    gl_Position = projection * view * model * totalPosition;

    TexCoords = aTexCoords;
}
