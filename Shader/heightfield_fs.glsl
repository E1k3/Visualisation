#version 330 core

in vec2 uv;
out vec4 color;

uniform sampler2D tex;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;

vec3 palette(float x)
{
	return vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f));
}

void main(void)
{
	color = vec4(palette(texture(tex, uv).r), 1.0f);
}
