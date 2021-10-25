#version 450

layout(binding = 0, std140) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform PushConsts2 {
	vec4 tt;
} pushConsts2;
layout(push_constant) uniform PushConsts {
	vec4 lightPos[6];
} pushConsts;


void main()
{
    gl_Position = ((ubo.proj * ubo.view) * ubo.model) * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
