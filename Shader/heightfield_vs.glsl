#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float height;
layout(location = 2) in float color;

out vec2 uv;

void main()
{
	gl_Position = vec4(pos, 0.f, 1.f);
	uv = pos/2.0f + 0.5f;
}
