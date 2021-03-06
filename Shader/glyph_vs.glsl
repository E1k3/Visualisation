#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float mean;
layout(location = 2) in float dev;

out vec2 gs_pos;
out vec2 gs_data;
out float gs_indicator;

uniform vec4 bounds;
uniform vec4 highlight_area;

void main()
{
	gs_pos = pos;
	gs_data = vec2((mean - bounds.x) / (bounds.y - bounds.x), (dev - bounds.z) / (bounds.w - bounds.z));
	gs_indicator = 1.f
			+ 200.f * float(pos.x >= highlight_area.x && pos.y >= highlight_area.y && pos.x < highlight_area.z && pos.y < highlight_area.w)
			- 2.f * float(mean == 0.f && dev == 0.f);
}
