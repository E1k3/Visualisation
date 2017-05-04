#version 450 core

in vec2 uv;
out vec4 color;

uniform sampler2D tex;

void main(void)
{
	float pi = 3.141592f;
	float x = texture(tex, uv).r;
	vec3 col = vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f));
	color = vec4(col, 1.0f);
}
