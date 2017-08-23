#version 330

in vec2 uv;

out vec4 color;

uniform sampler2D tex;

void main()
{
	color = vec4(vec3(1.f), texture(tex, uv).r);
}
