#version 450 core
layout (set = 0, binding = 1) uniform texture2D depthImg;
layout (set = 0, binding = 2) uniform sampler samplerColorMap;
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;
void main()
{
    vec2 Uv = uv;
    //Uv.x -= 0.5f;
    float a = texture(sampler2D(depthImg,samplerColorMap), Uv).x;
    outColor = vec4(a,a,a,1.0f);
}
