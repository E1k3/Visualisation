#version 330 core

in vec2 fs_uv;
in vec2 fs_data; // (mean, dev)

out vec4 color;

uniform sampler2D mask;

vec3 palette(float x);

void main()
{
	if(fs_data.x < 0.f && fs_data.y < 0.f)
		discard;

	vec3 mask = texture(mask, fs_uv).rgb;
	color = vec4(palette(mask.r*(fs_data.x + fs_data.y) + mask.g*fs_data.x + mask.b*(fs_data.x - fs_data.y)), 1.f);
}
