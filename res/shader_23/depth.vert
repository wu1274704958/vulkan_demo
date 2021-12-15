#version 450 core
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec2 inPosition;
layout(location = 2) in vec2 inUv;

layout(location = 1) out vec2 uv;

void main()
{
    gl_Position = vec4(inPosition, 0.0, 1.0);
    uv = inUv;
}
