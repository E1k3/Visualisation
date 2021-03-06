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

	if(fs_indicator > 50.f)
		color = color * .2f + vec4(0.f, .3f, .5f, 1.f);
}
