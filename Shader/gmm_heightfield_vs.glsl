#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in vec4 mean;
layout(location = 2) in vec4 var;
layout(location = 3) in vec4 weight;

out float fs_intensity;
out float fs_indicator;

uniform mat4 mvp;
uniform vec4 bounds;
uniform float time;

void main()
{
	float t = abs((time-int(time)-.5f)*2.f);
	vec4 norm_mean = (mean-bounds.x)/(bounds.y-bounds.x);
	vec4 norm_var = (var-bounds.z)/(bounds.w-bounds.z);

	vec4 weight_ = vec4(weight.x, weight.x+weight.y, weight.x+weight.y+weight.z, weight.x+weight.y+weight.z+weight.w);

	float mean_ = norm_mean.x * float(            t < weight_.x) +
	        norm_mean.y * float(t >= weight_.x && t < weight_.y) +
	        norm_mean.z * float(t >= weight_.y && t < weight_.z) +
	        norm_mean.w * float(t >= weight_.z);
	float var_ = norm_var.x * float(            t < weight_.x) +
	        norm_var.y * float(t >= weight_.x && t < weight_.y) +
	        norm_var.z * float(t >= weight_.y && t < weight_.z) +
	        norm_var.w * float(t >= weight_.z);

	gl_Position = mvp*vec4(pos, mean_, 1.f);
	fs_intensity = var_;
	fs_indicator = 1.f - 1000.f * float(mean.x == 0.f && var.x == 0.f && weight.x == 1.f);
}
