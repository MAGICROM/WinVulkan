#version 450

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 Uvs;
layout(binding = 0) uniform UBO {mat4 matrice;} mat;

vec2 UVs[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);

vec2 positions[6] = vec2[](
    vec2(-0.5, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5),
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(0.5, 0.5)
);

vec3 colors[6] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = mat.matrice * vec4( positions[gl_VertexIndex], 0.0, 1.0);
    Uvs = UVs[gl_VertexIndex];
    fragColor = colors[gl_VertexIndex];
}