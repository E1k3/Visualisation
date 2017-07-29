#version 330

layout(location = 0) in vec2 pos;

out float fs_intensity;

uniform vec2 origin;
uniform vec2 scale;

void main()
{
	gl_Position = vec4(origin + (pos * scale), 0.f, 1.f);
	fs_intensity = pos.x;
}
