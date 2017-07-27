#version 330

layout(location = 0) in vec4 pos;

out vec2 uv;

void main()
{
	gl_Position = vec4(pos.xy, 0.f, 1.f);
	uv = pos.zw;
}
