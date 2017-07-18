#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in vec4 mean;
layout(location = 2) in vec4 var;
layout(location = 3) in vec4 weight;

out float intensity;

uniform mat4 mvp;
uniform vec4 bounds;
uniform float time;

void main()
{
	// TODO:ANIMATE SMOOTHLY (INTERPOLATE) use cos(time) or similar as t
	float t = abs((time-int(time)-.5f)*2.f);
	vec4 _mean = (mean-bounds.x)/bounds.y/2;	// /2 to reduce peak height
	vec4 _var = (var-bounds.z)/bounds.w;
	float mean_, var_;
	if(t < weight.x)
	{
		mean_ = _mean.x;
		var_ = _var.x;
	}
	else if(t < weight.x + weight.y)
	{
		mean_ = _mean.y;
		var_ = _var.y;
	}
	else if(t < weight.x + weight.y + weight.z)
	{
		mean_ = _mean.z;
		var_ = _var.z;
	}
	else
	{
		mean_ = _mean.w;
		var_ = _var.w;
	}
	gl_Position = mvp*vec4(pos, mean_, 1.f);
	intensity = var_ - 10 * float(mean.x == 0.f && var.x == 0.f && weight.x == 1.f);
}
