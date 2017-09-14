#version 330 core

in vec2 fs_uv;
in vec2 fs_data; // (mean, dev)
in float fs_indicator;

out vec4 color;

vec3 palette(float x);

void main()
{
	if(fs_indicator < 0.f)
		discard;

	float distance = length(vec2(.5f) - fs_uv);
	const float rad_in = 0.325735007935279947724256415225564669717257072129485134758f;
	const float rad_out = 0.460658865961780639020326194709186244185836493768324417982f;

	color = vec4(palette((fs_data.x + fs_data.y) * float(distance < rad_in)
	                     + fs_data.x * float(distance >= rad_in && distance < rad_out)
	                     + (fs_data.x - fs_data.y) * float (distance >= rad_out)), 1.f);
	if(fs_indicator > 2.f)
		color = vec4(.5f, .5f, 1.f, 1.f);
}
