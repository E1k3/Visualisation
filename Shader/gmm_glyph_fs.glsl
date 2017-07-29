#version 330 core

smooth in vec2 uv;
flat in vec4 fs_mean; // (mean, dev)
flat in vec4 fs_var;
flat in vec4 fs_weight;

out vec4 color;

uniform sampler2D mask;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x);

void main()
{
	if(fs_weight.x < 0.f)
		discard;

	vec2 uv_ = uv - ivec2(uv);

	vec4 weightsum = vec4(fs_weight.x, fs_weight.x+fs_weight.y, fs_weight.x+fs_weight.y+fs_weight.z, fs_weight.x+fs_weight.y+fs_weight.z+fs_weight.w) * pi * 2.f;
	vec3 mask = texture(mask, uv_).rgb;
	vec4 comps = mask.r*(fs_mean + fs_var) + mask.g*fs_mean + mask.b*(fs_mean - fs_var);

	float angle = acos((.5f-uv_.y) / length(uv_-vec2(.5f))) * (float(.5f-uv_.x < 0.f)*2 - 1) + pi;

	color = vec4(palette(comps.x * float(                        angle < weightsum.x) +
	                     comps.y * float(angle >= weightsum.x && angle < weightsum.y) +
	                     comps.z * float(angle >= weightsum.y && angle < weightsum.z) +
	                     comps.w * float(angle >= weightsum.z)), 1.f);
}
