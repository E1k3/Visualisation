#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in vec4 mean;
layout(location = 2) in vec4 var;
layout(location = 3) in vec4 weight;

out vec2 gs_pos;
out vec4 gs_mean;
out vec4 gs_var;
out vec4 gs_weight;

uniform mat4 mvp;
uniform vec4 bounds;

void main()
{
	gs_pos = pos;
	gs_mean = (mean-bounds.x) / bounds.y;
	gs_var = sqrt(var) / (bounds.y - bounds.x);
	gs_weight = weight - 10 * float(/*weight.z == 0.f || */mean.x == 0.f && var.x == 0.f && weight.x == 1.f);
}
