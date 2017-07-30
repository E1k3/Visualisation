#version 330 core

in float fs_intensity;
out vec4 color;

uniform int divisions;
uniform ivec2 viewport;
uniform vec2 scale;

vec3 palette(float x);
void main()
{
	color = vec4(palette(fs_intensity), 1.f);
	float distance = 1 - abs(mod(fs_intensity * divisions-1, 1.f)*2.f - 1.f);
	if(distance * (scale.x / float(divisions*2)) * float(viewport.x) < 1)
		color = vec4(1.f);
}
