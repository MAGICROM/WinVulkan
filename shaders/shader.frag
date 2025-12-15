#version 450

layout (binding = 1) uniform sampler2D samplerColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 Uvs;

layout(location = 0) out vec4 outColor;

vec4 sampleTexture(vec2 uv)
{
    vec4 c00 = vec4( vec3(1.0,0.0,0.0), 1.0);
    vec4 c01 = vec4( vec3(1.0,0.5,0.0), 1.0);
    vec4 c02 = vec4( vec3(0.18,0.0,0.0), 1.0);
    vec4 c03 = vec4( vec3(1.0,0.0,0.20), 1.0);

    vec4 b0 = mix(c00, c01, uv.x);
    vec4 b1 = mix(c02, c03, uv.x);

    vec4 p0 = mix(b0, b1, uv.y);

    return p0;
}

void main() {
     outColor = texture(samplerColor,Uvs);
}