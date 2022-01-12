#version 450 core
layout (set = 0, binding = 1) uniform samplerCube samplerColorMap;
layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 Position;
layout(location = 2) in vec2 Uv;
layout(location = 3) in mat4 worldMat;
layout(location = 0) out vec4 outColor;

float saturate (float x)
{
    return min(1.0, max(0.0,x));
}

vec3 saturate (vec3 x)
{
    return min(vec3(1.,1.,1.), max(vec3(0.,0.,0.),x));
}

vec3 bump3y (vec3 x, vec3 yoffset)
{
	vec3 y = vec3(1.,1.,1.) - x * x;
	y = saturate(y-yoffset);
	return y;
}

//  Zucconi 6
vec3 spectral_zucconi6 (float w)
{
	// w: [400, 700]
	// x: [0,   1]
	float x = saturate((w - 400.0)/ 300.0);

	const vec3 c1 = vec3(3.54585104, 2.93225262, 2.41593945);
	const vec3 x1 = vec3(0.69549072, 0.49228336, 0.27699880);
	const vec3 y1 = vec3(0.02312639, 0.15225084, 0.52607955);

	const vec3 c2 = vec3(3.90307140, 3.21182957, 3.96587128);
	const vec3 x2 = vec3(0.11748627, 0.86755042, 0.66077860);
	const vec3 y2 = vec3(0.84897130, 0.88445281, 0.73949448);

	return
		bump3y(c1 * (x - x1), y1) +
		bump3y(c2 * (x - x2), y2) ;
}

vec3 lightDiffraction(vec3 lightDir, vec3 viewDir, vec3 rColor){
	vec2 uv = Uv * 2 - 1;
	vec2 uv_orthogonal  = normalize( uv );
	vec3 uv_tangent  = vec3( -uv_orthogonal.y, 0, uv_orthogonal.x);
	vec4 word_tangent = normalize( worldMat * vec4(uv_tangent, 0));
	
	vec3 L = lightDir;
	vec3 V = viewDir;
	vec3 T = word_tangent.xyz;
	
	int d = 1600;
	
	float cos_ThetaL = dot(L, T);
    float cos_ThetaV = dot(V, T);
    float u = abs(cos_ThetaL - cos_ThetaV);
	
	if(u == 0.){
		return rColor;
	}
	
	vec3 color = vec3(0,0,0);
    for (int n = 1; n <= 7; n++)
    {
        float wavelength = u * d / n;
        
        color += spectral_zucconi6(wavelength);
        
    }
    color = saturate(color);
    
    return color;
}

void main()
{
    vec2 uv = Uv * 2 - 1;
    if(length(uv) < 0.2){
    		discard;
    		return;
    }
   

    vec3 view_pos = vec3(0,0,0);
	vec3 light_pos = vec3(10,10,3);
	vec3 pos = (worldMat * vec4(Position,1.0f)).rgb;
	
    vec3 lightDir = normalize(light_pos - pos);
	vec3 viewDir = normalize( view_pos - pos );
    
    vec3 I = - viewDir;
    vec3 R = reflect(I,normalize(normal));


    vec3 obj_color = texture(samplerColorMap, R).rgb;

	obj_color += lightDiffraction(lightDir, viewDir , obj_color);

    outColor = vec4(obj_color,1);//texture(samplerColorMap, uv);
}
