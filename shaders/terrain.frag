#version 330 core

in vec3 FragPos;
in vec3 Normal;
in float Height;

out vec4 FragColor;

uniform vec3 uLightDir = normalize(vec3(0.5, 1.0, 0.3));
uniform float uWaterLevel = 0.0;        // Y-coordinate of water surface
uniform vec3 uWaterColor = vec3(0.0, 0.3, 0.6); // blue water
uniform float uWaterOpacity = 0.9;      // 0 = fully transparent, 1 = opaque

vec3 getColor(float h) {
    if (h < -30) return vec3(0.0, 0.0, 0.6);      // deep water
    else if (h < 0) return vec3(0.0, 0.5, 1.0);   // shallow water
    else if (h < 30) return vec3(0.2, 0.8, 0.2);   // grass
    else if (h < 100) return vec3(0.5, 0.5, 0.5);   // rock
    else return vec3(1.0, 1.0, 1.0);               // snow
}

void main() {
    vec3 N = normalize(Normal);
    float diff = max(dot(N, normalize(uLightDir)), 0.0);

    // Terrain base color by height
    vec3 baseColor = getColor(Height);
    vec3 litColor = baseColor * diff;

    // Water overlay for heights below water level
    if (Height < uWaterLevel) {
        float alpha = uWaterOpacity; // 0 = fully transparent, 1 = opaque
        litColor = mix(litColor, uWaterColor, alpha);
    }

    FragColor = vec4(litColor, 1.0);
}

