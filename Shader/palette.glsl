vec3 _palette_interpolate_(float x)
{
	// hot
	vec4 hot[5] = vec4[5](
				vec4(0.f, 0.f, 0.f, 0.f),
				vec4(.39f, .7f, .13f, .13f),
				vec4(.58f, .9f, .41f, .02f),
				vec4(.84f, .93f, .82f, .08f),
				vec4(1.f, 1.f, 1.f, 1.f));
	// kindlmann
	vec4 kindlmann[11] = vec4[11](
	            vec4(.0f, 0.f, 0.f, 0.f),
	            vec4(.1f, .18f, .015f, .3f),
	            vec4(.2f, .25f, .027f, .57f),
	            vec4(.3f, .031f, .25f, .65f),
	            vec4(.4f, .02f, .42f, .42f),
	            vec4(.5f, .027f, .54f, .27f),
	            vec4(.6f, .031f, .66f, .1f),
	            vec4(.7f, .33f, .76f, .035f),
	            vec4(.8f, .77f, .81f, .04f),
	            vec4(.9f, .99f, .86f, .77f),
	            vec4(1.f, 1.f, 1.f, 1.f));

//	vec4 points[] = hot;
	vec4 points[] = kindlmann;


	for(int i = 0; i < points.length(); ++i)
		if(x < points[i].x)
			return mix(points[i-1].yzw, points[i].yzw, (x - points[i-1].x) / (points[i].x - points[i-1].x));

	return points[points.length()-1].yzw;
}

vec3 palette(float x)
{
	const float _palette_pi_ = 3.141592653589793238462643383279502884197169399375105820974f;
	const float _sqrt_three_ = 1.732050807568877293527446341505872366942805253810380628055f;
	x = clamp(x, 0.f, 1.f);

	vec3 color = vec3(0.f);
//	color = vec3(x);                   // black->white
//	color = vec3(x, 0.f, 0.f);         // black->red
//	color = vec3(x*.7f+.15f, .15f, .15f); // dark red-grey->matt red
//	color = vec3(0.f, x, 0.f);         // black->green
//	color = vec3(.15f, x*.7f+.15f, .15f); // dark green-grey->matt green
//	color = vec3(0.f, 0.f, x);         // black->blue
//	color = vec3(.15f, .15f, x*.7f+.15f); // dark blue-grey->matt blue
//	color = vec3(x, 1.f-x, 0.f);
//	color = vec3(x, 0.f, 1.f-x);
//	color = vec3(1.f-x, x, 0.f);
//	color = vec3(0.f, x, 1.f-x);
//	color = vec3(1.f-x, 0.f, x);
//	color = vec3(0.f, 1.f-x, x);

	color = _palette_interpolate_(x*.8f + .1f);

//	color = vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * _palette_pi_ * x), 0.f, 1.f));

	return color;
}
