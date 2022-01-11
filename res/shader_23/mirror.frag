#version 450 core
layout (set = 0, binding = 1) uniform samplerCube samplerColorMap;
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
void main()
{
    vec3 view_pos = vec3(0,0,0);
    vec3 I = normalize( pos - view_pos );
    vec3 R = reflect(I,normalize(normal));


    vec3 obj_color = texture(samplerColorMap, R).rgb;
    outColor = vec4(obj_color,1);//texture(samplerColorMap, uv);
}
