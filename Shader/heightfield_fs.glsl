#version 330 core

in float intensity;

out vec4 color;

vec3 palette(float x);

void main()
{
	color = vec4(palette(intensity), 1.0f);
	if(intensity < 0.f)
		discard;
}
