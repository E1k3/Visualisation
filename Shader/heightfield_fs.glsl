#version 330 core

in float intensity;

out vec4 color;

vec3 palette(float x);

void main()
{
	if(intensity < 0.f)
		discard;
	color = vec4(palette(intensity), 1.0f);
}
