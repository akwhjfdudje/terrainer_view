#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out float Height; // NEW: pass height

uniform mat4 uView;
uniform mat4 uProj;

void main() {
    FragPos = aPos;
    Normal = aNormal;
    Height = aPos.y; // pass vertex height
    gl_Position = uProj * uView * vec4(aPos, 1.0);
}

