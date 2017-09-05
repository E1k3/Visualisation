#version 330 core

in float fs_intensity;
in float fs_indicator;

out vec4 color;

vec3 palette(float x);

void main()
{
	if(fs_indicator < 0.f)
		discard;

	color = vec4(palette(fs_intensity), 1.0f);

	if(fs_indicator > 2.f)
		color = vec4(1.f);
}
