#version 330 core

smooth in vec2 uv;
flat in vec2 data; // (mean, dev)

out vec4 color;

uniform sampler2D mask;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x)
{
	return clamp(vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f)), 0.f, 1.f);
}

void main()
{
	if(data.x < 0.f && data.y < 0.f)
		discard;

	vec3 mask = texture(mask, uv).rgb;
	color = vec4(palette(mask.r*(data.x + data.y) + mask.g*data.x + mask.b*(data.x - data.y)), 1.f);
}
