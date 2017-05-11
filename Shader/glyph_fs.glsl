#version 330 core

flat in vec2 data;
smooth in vec2 position;
flat in vec2 flatpos;

out vec4 color;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x)
{
	return vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f));
}

void main(void)
{
	color = vec4(palette(data.y), 1.f);
	if(distance(position, flatpos) < .75f/96.f)
		color = vec4(vec3(1.f)-palette(data.y), 1.f);
	if(distance(position, flatpos) < .5f/96.f)
		color = vec4(palette(data.x), 1.f);
}
