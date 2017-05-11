#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float var;
layout(location = 2) in float avg;

flat out vec2 data;
smooth out vec2 position;
flat out vec2 flatpos;

uniform mat4 mvp;

void main()
{
	    gl_Position = mvp*vec4(pos, 0.f, 1.f);
		position = pos*0.5f + vec2(.5f);
		flatpos = position;
		data = vec2(avg, var);
}
