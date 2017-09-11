#version 330 core

flat in int instance_id;

out vec4 color_out;

uniform vec4 colors[64];

void main()
{
	color_out = colors[instance_id];
}
