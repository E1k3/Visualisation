#version 330 core

layout(location = 0) in vec3 pos;

flat out int instance_id;

uniform mat4 mvp;
uniform vec3 translations[64];

void main()
{
	gl_Position = mvp * vec4(pos + translations[gl_InstanceID], 1.f);
	instance_id = gl_InstanceID;
}
