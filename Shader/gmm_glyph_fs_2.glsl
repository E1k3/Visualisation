#version 330 core

smooth in vec2 uv;
flat in vec4 mean_; // (mean, dev)
flat in vec4 var_;
flat in vec4 weight_;

out vec4 color;

uniform sampler2D mask;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x)
{
	return clamp(vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f)), 0.f, 1.f);
}

void main()
{
	if(weight_.x < 0.f)
		discard;

	vec3 mask = texture(mask, uv).rgb;

	float angle = acos((uv.y-.5f) / length(uv-vec2(.5f))) * (float(uv.x-.5f < 0.f)*2 - 1) + pi;
	vec4 comps = mask.r*(mean_ + var_) + mask.g*mean_ + mask.b*(mean_ - var_);

	vec4 weight = vec4(weight_.x, weight_.x+weight_.y, weight_.x+weight_.y+weight_.z, weight_.x+weight_.y+weight_.z+weight_.w);
	weight *= pi * 2.f;
	color = vec4(palette(comps.x * float(						angle < weight.x) +
	                     comps.y * float(angle >= weight.x &&	angle < weight.y) +
	                     comps.z * float(angle >= weight.y &&	angle < weight.z) +
	                     comps.w * float(angle >= weight.z)), 1.f);
}
