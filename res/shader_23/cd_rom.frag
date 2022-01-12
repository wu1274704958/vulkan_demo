#version 450 core
layout (set = 0, binding = 1) uniform samplerCube samplerColorMap;
layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 Position;
layout(location = 2) in vec2 Uv;
layout(location = 3) in mat4 worldMat;
layout(location = 0) out vec4 outColor;
void main()
{
    vec2 uv = Uv * 2 - 1;
    if(length(uv) < 0.2){
    		discard;
    		return;
    }
    vec3 view_pos = vec3(0,0,0);
    vec3 pos = (worldMat * vec4(Position,1.0f)).rgb;
    vec3 I = normalize( pos - view_pos );
    vec3 R = reflect(I,normalize(normal));


    vec3 obj_color = texture(samplerColorMap, R).rgb;
    outColor = vec4(obj_color,1);//texture(samplerColorMap, uv);
}
