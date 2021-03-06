#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec2 gs_pos[];
in vec2 gs_data[];
in float gs_indicator[];

out vec2 fs_uv;
out vec2 fs_data;
out float fs_indicator;

uniform mat4 mvp;
uniform ivec2 field_size;

void main()
{
	fs_data = gs_data[0];
	fs_indicator = gs_indicator[0];
	vec4 pos = vec4(gs_pos[0], 0.f, 1.f);
	vec2 glyph_size = 2.f / vec2(field_size - 1);

	fs_uv = vec2(0.f, 0.f);
	gl_Position = mvp * pos;
	EmitVertex();

	fs_uv = vec2(1.f, 0.f);
	gl_Position = mvp * (pos + vec4(glyph_size.x, 0.f, 0.f, 0.f));
	EmitVertex();

	fs_uv = vec2(0.f, 1.f);
	gl_Position = mvp * (pos + vec4(0.f, glyph_size.y, 0.f, 0.f));
	EmitVertex();

	fs_uv = vec2(1.f, 1.f);
	gl_Position = mvp * (pos + vec4(glyph_size , 0.f, 0.f));
	EmitVertex();

	EndPrimitive();
}
