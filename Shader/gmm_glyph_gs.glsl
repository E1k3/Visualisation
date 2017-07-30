#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec2 gs_pos[];
in vec4 gs_mean[];
in vec4 gs_var[];
in vec4 gs_weight[];

out vec2 fs_uv;
out vec4 fs_mean;
out vec4 fs_var;
out vec4 fs_weight;

uniform mat4 mvp;
uniform ivec2 field_size;

void main()
{
	fs_mean = gs_mean[0];
	fs_var = gs_var[0];
	fs_weight = gs_weight[0];

	vec4 pos = vec4(gs_pos[0], 0.f, 1.f);
	vec2 glyph_field_size = 2.f / vec2(field_size - 1);

	fs_uv = vec2(0.f, 0.f);
	gl_Position = mvp * pos;
	EmitVertex();

	fs_uv = vec2(1.f, 0.f);
	gl_Position = mvp * (pos + vec4(glyph_field_size.x, 0.f, 0.f, 0.f));
	EmitVertex();

	fs_uv = vec2(0.f, 1.f);
	gl_Position = mvp * (pos + vec4(0.f, glyph_field_size.y, 0.f, 0.f));
	EmitVertex();

	fs_uv = vec2(1.f, 1.f);
	gl_Position = mvp * (pos + vec4(glyph_field_size , 0.f, 0.f));
	EmitVertex();

	EndPrimitive();
}
