#version 330

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 mean;
layout(location = 2) in vec4 var;
layout(location = 3) in vec4 weight;

smooth out vec2 uv;
flat out vec4 fs_mean;
flat out vec4 fs_var;
flat out vec4 fs_weight;

uniform ivec2 gridsize = ivec2(191, 95);
uniform mat4 mvp;
uniform vec4 bounds;

void main()
{
	gl_Position = mvp*vec4(pos.xy, 0.f, 1.f);

	uv = (pos.xy*.5f+.5f)*gridsize;

	fs_mean = (mean-bounds.x) / bounds.y;
	fs_var = (var-bounds.z) / bounds.w;
	fs_weight = weight - 10 * float(/*weight.z == 0.f || */mean.x == 0.f && var.x == 0.f && weight.x == 1.f);
}
