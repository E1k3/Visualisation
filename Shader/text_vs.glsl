#version 330

layout(location = 0) in vec4 pos;

out vec2 uv;

uniform vec2 origin;
uniform ivec2 viewport;

void main()
{
	gl_Position = vec4(origin + (pos.xy / viewport), 0.f, 1.f);
	uv = pos.zw;
}
