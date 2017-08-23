#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in vec4 mean;
layout(location = 2) in vec4 var;
layout(location = 3) in vec4 weight;

out vec2 gs_pos;
out vec4 gs_mean;
out vec4 gs_var;
out vec4 gs_weight;
out float gs_indicator;

uniform mat4 mvp;
uniform vec4 bounds;
uniform vec4 highlight_area;

void main()
{
	gs_pos = pos;
	gs_mean = (mean-bounds.x) / (bounds.y-bounds.x);
	gs_var = sqrt(var) / (bounds.y - bounds.x);
	gs_weight = weight;
	gs_indicator = 1.f
			+ 2.f * float(pos.x >= highlight_area.x && pos.y >= highlight_area.y && pos.x <= highlight_area.z && pos.y <= highlight_area.w)
			- 100.f * float(mean.x == 0.f && var.x == 0.f && weight.x == 1.f);
}
