#version 330 core

in float intensity;
in float indicator;

out vec4 color;

vec3 palette(float x);

void main()
{
	if(indicator < 0.f)
		discard;
	color = vec4(palette(intensity), 1.0f);
}
