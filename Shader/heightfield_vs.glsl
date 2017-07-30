#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in float mean;
layout(location = 2) in float var;

out float intensity;

uniform mat4 mvp;
uniform vec4 bounds;

void main()
{
	float mean_ = (mean-bounds.x)/bounds.y;
	gl_Position = mvp*vec4(pos, mean_, 1.f);
	intensity = (var-bounds.z)/bounds.w - 10 * float(mean == 0.f && var == 0.f);
}
