#version 450 core
layout (set = 0, binding = 1) uniform samplerCube samplerColorMap;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec3 uv;
void main()
{
    outColor = texture(samplerColorMap, uv);
}
