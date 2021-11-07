#version 450 core
layout (set = 0, binding = 1) uniform sampler2D samplerColorMap;
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;

void main()
{
    outColor = vec4(fragColor, 1.0);
}
