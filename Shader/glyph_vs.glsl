#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float mean;
layout(location = 2) in float var;

out vec2 gs_pos;
out vec2 gs_data;

uniform mat4 mvp;
uniform vec4 bounds;

void main()
{
	gs_pos = pos;
	gs_data = vec2((mean - bounds.x) / (bounds.y-bounds.x), var / (bounds.y - bounds.x)) - 10 * float(mean == 0.f && var == 0.f);
}
