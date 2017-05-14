#version 330 core

in vec2 uv;

out vec4 color;

uniform ivec2 size;

uniform sampler2D avg;
uniform sampler2D var;

const float pi = 3.141592653589793238462643383279502884197169399375105820974f;
vec3 palette(float x)
{
	return vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * pi * x), 0.f, 1.f));
}

const float r_outer = 0.460658865961780639020326194709186244185836493768324417982f;
const float r_inner = 0.325735007935279947724256415225564669717257072129485134758f;
void main(void)
{
	vec2 texcoords = floor(uv*vec2(192, 96))/vec2(192, 96);
	vec2 mid = texcoords + vec2(.5f/192, .5f/96);
	float normdist = length((mid - uv) * vec2(192, 96));
	if(normdist < r_inner)
	{
		color = vec4(palette(texture(avg, texcoords).r), 1.f);
	}
	else if(normdist < r_outer)
	{
		color = vec4(palette(texture(var, texcoords).r), 1.f);
	}
	else
	{
		color = vec4(1.f - palette(texture(var, texcoords).r), 1.f);
	}
}
