#version 330 core

smooth in vec2 uv;
flat in vec2 data;

out vec4 color;

uniform uvec2 size;

uniform sampler2D mask;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x)
{
	return vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f));
}

vec3 palette2(float x)
{
	return vec3(x, x, x);
}

void main(void)
{
	vec2 fsize = vec2(size.x, size.y);
	vec2 origin = floor(uv*size)/size;
	vec2 texpos = (uv-origin)*size;
	vec3 mask = texture(mask, texpos).rgb;
	color = vec4(palette(mask.x*(data.x + data.y) + mask.y*data.x + mask.z*(data.x - data.y)), 1.f);
}
