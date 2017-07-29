#version 330 core

in float fs_intensity;
out vec4 color;

vec3 palette(float x);
void main()
{
	color = vec4(palette(fs_intensity), 1.f);
}
