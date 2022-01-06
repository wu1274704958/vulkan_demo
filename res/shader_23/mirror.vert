#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "test.glsl"


layout(binding = 0, std140) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform model { // 这里接住数据
	mat4 model;
} model_mat;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 pos;


void main()
{
    gl_Position = ubo.proj * ubo.view * model_mat.model * vec4(inPosition, 1.0);
    pos = (ubo.view * model_mat.model * vec4(inPosition, 1.0)).rgb;
}
