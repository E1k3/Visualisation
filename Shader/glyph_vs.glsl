#version 330

layout(location = 0) in vec2 pos;

out vec2 uv;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp*vec4(pos, 0.f, 1.f);
	uv = pos*.5f + .5f;
}
