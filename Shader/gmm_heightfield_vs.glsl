#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in vec4 mean;
layout(location = 2) in vec4 dev;
layout(location = 3) in vec4 weight;

out float fs_intensity;
out float fs_indicator;

uniform mat4 mvp;
uniform vec4 bounds;
uniform float time;
uniform vec4 highlight_area;

void main()
{
	float time_ = time * .2f;
//	float t = abs((time_-int(time_)-.5f)*2.f);
	float t = mod(time_, 1.f);
	vec4 norm_mean = (mean - bounds.x)/(bounds.y - bounds.x);
	vec4 norm_dev = (dev - bounds.z)/(bounds.w - bounds.z);

	vec4 weight_ = vec4(weight.x, weight.x+weight.y, weight.x+weight.y+weight.z, weight.x+weight.y+weight.z+weight.w);

	float mean_ = norm_mean.x * float(            t < weight_.x) +
	        norm_mean.y * float(t >= weight_.x && t < weight_.y) +
	        norm_mean.z * float(t >= weight_.y && t < weight_.z) +
	        norm_mean.w * float(t >= weight_.z);
	float dev_ = norm_dev.x * float(            t < weight_.x) +
	        norm_dev.y * float(t >= weight_.x && t < weight_.y) +
	        norm_dev.z * float(t >= weight_.y && t < weight_.z) +
	        norm_dev.w * float(t >= weight_.z);

	gl_Position = mvp*vec4(pos, mean_, 1.f);
	fs_intensity = dev_;
	fs_indicator = 1.f
			+ 200.f * float(pos.x >= highlight_area.x && pos.y >= highlight_area.y && pos.x <= highlight_area.z && pos.y <= highlight_area.w)
			- 100.f * float(mean.x == 0.f && dev.x == 0.f && weight.x == 1.f);
}
