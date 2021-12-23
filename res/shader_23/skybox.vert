#version 450 core
#extension GL_GOOGLE_include_directive : enable


layout(binding = 0, std140) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform model { // 这里接住数据
	mat4 model;
} model_mat;


layout(location = 0) in vec3 inPosition;

layout(location = 1) out vec3 uv;


void main()
{
    gl_Position = ((ubo.proj * ubo.view) * model_mat.model) * vec4(inPosition, 1.0);

    gl_Position.w = gl_Position.z;

    uv = inPosition;
}
