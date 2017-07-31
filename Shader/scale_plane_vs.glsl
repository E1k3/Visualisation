#version 330 core

layout(location = 0) in vec2 pos;

uniform mat4 mvp;
uniform int count;

void main()
{
	gl_Position = mvp*vec4(pos, float(gl_InstanceID) / (count-1), 1.f);
}
