#version 330 core

in vec2 fs_uv;
in vec4 fs_mean;
in vec4 fs_dev;
in vec4 fs_weight;
in float fs_indicator;

out vec4 color;

uniform sampler2D mask;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x);

void main()
{
	if(fs_indicator < 0.f)
		discard;

	vec4 weightsum = vec4(fs_weight.x, fs_weight.x+fs_weight.y, fs_weight.x+fs_weight.y+fs_weight.z, fs_weight.x+fs_weight.y+fs_weight.z+fs_weight.w) * pi * 2.f;
	vec3 mask = texture(mask, fs_uv).rgb;
	vec4 comps = mask.r*(fs_mean + fs_dev) + mask.g*fs_mean + mask.b*(fs_mean - fs_dev);

	float angle = acos((.5f-fs_uv.y) / length(fs_uv-vec2(.5f))) * (float(.5f-fs_uv.x < 0.f)*2 - 1) + pi;

	color = vec4(palette(comps.x * float(                        angle < weightsum.x) +
	                     comps.y * float(angle >= weightsum.x && angle < weightsum.y) +
	                     comps.z * float(angle >= weightsum.y && angle < weightsum.z) +
	                     comps.w * float(angle >= weightsum.z)), 1.f);
	if(fs_indicator > 2.f)
		color = vec4(1.f);
}
