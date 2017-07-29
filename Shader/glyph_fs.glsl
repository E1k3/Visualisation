#version 330 core

smooth in vec2 uv;
flat in vec2 data; // (mean, dev)

out vec4 color;

uniform sampler2D mask;

vec3 palette(float x);

void main()
{
	if(data.x < 0.f && data.y < 0.f)
		discard;

	vec3 mask = texture(mask, uv).rgb;
	color = vec4(palette(mask.r*(data.x + data.y) + mask.g*data.x + mask.b*(data.x - data.y)), 1.f);
}
