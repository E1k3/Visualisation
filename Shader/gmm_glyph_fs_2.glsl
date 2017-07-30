#version 330 core

in vec2 fs_uv;
in vec4 fs_mean;
in vec4 fs_var;
in vec4 fs_weight;

out vec4 color;

uniform sampler2D mask;

const float dot_radius = 0.2613616004385317f;	// pi * dot_radius^2 = 1 - pi * 0.5^2 = background area
vec3 palette(float x);

void main()
{
	if(fs_weight.x <= 0.f)
		discard;

	vec4 weightsum = vec4(fs_weight.x, fs_weight.x+fs_weight.y, fs_weight.x+fs_weight.y+fs_weight.z, fs_weight.x+fs_weight.y+fs_weight.z+fs_weight.w) * (1 - dot_radius) + dot_radius;
	float distance = length(vec2(.5f) - fs_uv)*2.f;

	int last = 0;
	for(;last < 3;)
	{
		if(fs_weight[last+1] == 0.f) break;
		else ++last;
	}

	color = vec4(palette((fs_mean.x - fs_var.x) * float(distance < dot_radius) +
	                     fs_mean.x            * float(distance < weightsum.x                 && distance >= dot_radius) +
	                     fs_mean.y            * float(distance < weightsum.y                 && distance >= weightsum.x) +
	                     fs_mean.z            * float(distance < weightsum.z                 && distance >= weightsum.y) +
	                     fs_mean.w            * float(distance < weightsum.w                 && distance >= weightsum.z) +
	                     (fs_mean[last] + fs_var[last]) * float(                                distance >= 1.f) ), 1.f);
}
