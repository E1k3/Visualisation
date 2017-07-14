#version 330 core

in float intensity;

out vec4 color;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x)
{
	return vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f));
}

void main()
{
	color = vec4(palette(intensity), 1.0f);
	if(intensity < 0.f)
		discard;
}
