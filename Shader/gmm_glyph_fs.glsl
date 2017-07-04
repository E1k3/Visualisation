#version 330 core

smooth in vec2 uv;
flat in vec4 mean_; // (mean, dev)
flat in vec4 var_;
flat in vec4 weight_;

out vec4 color;

uniform uvec2 size;
uniform sampler2D mask;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x)
{
	return clamp(vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f)), 0.f, 1.f);
}

void main()
{
	vec3 mask = texture(mask, uv).rgb;
	color = vec4(palette(mask.r*(mean_.x + var_.x) + mask.g*mean_.x + mask.b*(mean_.x - var_.x)), 1.f);
	if(uv.x+uv.y > 1.f && weight_.y > 0.f)
		color = vec4(palette(mask.r*(mean_.y + var_.y) + mask.g*mean_.y + mask.b*(mean_.y - var_.y)), 1.f);
}
