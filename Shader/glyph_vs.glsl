#version 330

layout(location = 0) in vec4 pos;
layout(location = 1) in float mean;
layout(location = 2) in float dev;

smooth out vec2 uv;
flat out vec2 data;

uniform mat4 mvp;
uniform vec4 bounds;

void main()
{
	gl_Position = mvp*vec4(pos.xy, 0.f, 1.f);
	uv = pos.zw;
	data = (vec2(mean, dev)-bounds.xz) / bounds.yw;
}
