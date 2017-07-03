#version 330

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 mean;
layout(location = 2) in vec4 var;
layout(location = 3) in vec4 weight;

smooth out vec2 uv;
flat out vec4 mean_;
flat out vec4 var_;
flat out vec4 weight_;

uniform mat4 mvp;
uniform vec4 bounds;

void main()
{
	gl_Position = mvp*vec4(pos.xy, 0.f, 1.f);
	uv = pos.zw;
	mean_ = (mean-bounds.x) / bounds.y;
	var_ = (var-bounds.z) / bounds.w;
	weight_ = weight;
}
