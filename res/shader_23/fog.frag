#version 450 core
layout (set = 0, binding = 1) uniform texture2D depthImg;
layout (set = 0, binding = 2) uniform sampler samplerColorMap;
layout (set = 0, binding = 3) uniform texture2D Img;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 uv;



void main()
{
    vec3 _FogColor = vec3(1,1,1);
    vec2 Uv = uv;
    float a = texture(sampler2D(depthImg,samplerColorMap), Uv).x;
    vec3 color = texture(sampler2D(Img,samplerColorMap),Uv).xyz;
    //step(0.6,a) * (a*a)
    outColor = vec4( mix(color, _FogColor, smoothstep (0.6, 1.0, a )) ,1.0);
}


