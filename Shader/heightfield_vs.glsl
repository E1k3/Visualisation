#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float mean;
layout(location = 2) in float dev;

out float fs_intensity;
out float fs_indicator;

uniform mat4 mvp;
uniform vec4 bounds;
uniform vec4 highlight_area;

void main()
{
	float mean_ = (mean - bounds.x)/(bounds.y - bounds.x);
	gl_Position = mvp*vec4(pos, mean_, 1.f);
	fs_intensity = (dev - bounds.z)/(bounds.w - bounds.z);
	fs_indicator = 1.f
	        + 200.f * float(pos.x >= highlight_area.x && pos.y >= highlight_area.y && pos.x < highlight_area.z && pos.y < highlight_area.w)
	        - 2.f * float(mean == 0.f && dev == 0.f);
}
