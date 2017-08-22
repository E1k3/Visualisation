#version 330 core

in vec2 fs_uv;
in vec2 fs_data; // (mean, dev)
in float fs_indicator;

out vec4 color;

uniform sampler2D mask;

vec3 palette(float x);

void main()
{
	if(fs_indicator < 0.f)
		discard;
	if(fs_indicator > 0.f)
	{

	}

	vec3 mask = texture(mask, fs_uv).rgb;
	color = vec4(palette(mask.r*(fs_data.x + fs_data.y) + mask.g*fs_data.x + mask.b*(fs_data.x - fs_data.y)), 1.f);
}
