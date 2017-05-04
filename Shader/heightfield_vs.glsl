#version 450

layout(location = 0) in vec2 pos;
layout(location = 1) in float height;

out vec2 uv;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp*vec4(pos, height*.2f, 1.f);
	uv = 0.5f- pos/2.f;
}
