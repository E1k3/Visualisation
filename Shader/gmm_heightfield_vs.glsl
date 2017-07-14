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
	vec4 mean_ = (mean-bounds.x)/bounds.y;
	vec4 var_ = (var-bounds.z)/bounds.w;
	gl_Position = mvp*vec4(pos, mean_*.5f, 1.f);
	intensity = var.x - 10 * float(mean.x == 0.f && var.x == 0.f && weight.x == 1.f);
}
