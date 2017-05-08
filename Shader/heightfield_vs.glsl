#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float height;
layout(location = 2) in float color;

out float col;

uniform mat4 mvp;

void main()
{
        gl_Position = mvp*vec4(pos, height*.5f, 1.f);
	col = color;
}
