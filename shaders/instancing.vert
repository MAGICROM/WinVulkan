#version 450

// Vertex attributes
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 modelview;
	vec4 lightPos;
	float locSpeed;
	float globSpeed;
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;
layout (location = 5) out vec3 outMaterial;

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

layout( push_constant ) uniform constants
{
	mat4 model;
	vec4 data;
} PushConstants;

void main() 
{
	outUV = vec3(inUV, PushConstants.data.x);
	gl_Position = ubo.projection * ubo.modelview * PushConstants.model * vec4( inPos.xyz, 1.0);
	mat3 m3viewmodel = mat3(ubo.modelview) * mat3(PushConstants.model);
	outNormal = m3viewmodel * inNormal;
	vec4 pos = ubo.modelview * PushConstants.model * vec4(inPos.xyz, 1.0);
	vec3 lPos = m3viewmodel * ubo.lightPos.xyz;
	outLightVec = lPos - pos.xyz;
	outViewVec = -pos.xyz;
	outColor = vec3(1.0,1.0,1.0);	
	outMaterial.x = 1.0;	
	outMaterial.y = 0.1;		
	outMaterial.z = 0.1;	
}
