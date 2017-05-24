#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float avg;
layout(location = 2) in float var;

out float col;

uniform mat4 mvp;
uniform vec4 bounds;

void main()
{
	float avg_ = (avg-bounds.x)/bounds.y;
	gl_Position = mvp*vec4(pos, avg_*.5f, 1.f);
	col = (var-bounds.z)/bounds.w;
}
