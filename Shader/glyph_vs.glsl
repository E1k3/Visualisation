#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float var;
layout(location = 2) in float avg;

smooth out vec2 uv;
flat out vec2 data;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp*vec4(pos, 0.f, 1.f);
	uv = pos*.5f + .5f;
	data = vec2(avg, var);
}
