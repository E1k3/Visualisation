#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float avg;
layout(location = 2) in float var;

smooth out vec2 uv;
flat out vec2 data;

uniform mat4 mvp;
uniform vec4 bounds;

void main()
{
	gl_Position = mvp*vec4(pos, 0.f, 1.f);
	uv = pos*.5f + .5f;
	data = (vec2(avg, var)-bounds.xz) / bounds.yw;
}
