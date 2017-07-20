#version 330 core

smooth in vec2 uv;
flat in vec4 mean_; // (mean, dev)
flat in vec4 var_;
flat in vec4 weight_;

out vec4 color;

uniform sampler2D mask;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x)
{
	return clamp(vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f)), 0.f, 1.f);
}

void main()
{
	if(weight_.x < 0.f)
		discard;

	vec2 uv_ = uv - ivec2(uv);



}
