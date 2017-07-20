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

	vec2 uv_ = uv - ivec2(uv);
	vec4 weight = vec4(weight_.x, weight_.x+weight_.y, weight_.x+weight_.y+weight_.z, weight_.x+weight_.y+weight_.z+weight_.w);
	float distance = length(vec2(.5f) - uv_)*2.f;

	color = vec4(palette((mean_.x - var_.x) * float(distance < weight.x/2.f) +
	                     mean_.x            * float(distance < weight.x                 && distance >= weight.x/2.f) +
	                     (mean_.y - var_.y) * float(distance < weight.x + weight_.y/2.f && distance >= weight.x) +
	                     mean_.y            * float(distance < weight.y                 && distance >= weight.x + weight_.y/2.f) +
	                     (mean_.z - var_.z) * float(distance < weight.y + weight_.z/2.f && distance >= weight.y) +
	                     mean_.z            * float(distance < weight.z                 && distance >= weight.y + weight_.z/2.f) +
	                     (mean_.w - var_.w) * float(distance < weight.z + weight_.w/2.f && distance >= weight.z) +
	                     mean_.w            * float(distance < weight.w                 && distance >= weight.z + weight_.w/2.f) +
	                     (mean_.x + var_.x) * float(                                       distance >= weight.w) ), 1.f);
}
