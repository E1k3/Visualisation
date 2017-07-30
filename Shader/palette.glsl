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
	color = vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * _palette_pi_ * x), 0.f, 1.f));
//	color = vec3(x, 1.f-x, 0.f);
//	color = vec3(x, 0.f, 1.f-x);
//	color = vec3(1.f-x, x, 0.f);
//	color = vec3(0.f, x, 1.f-x);
//	color = vec3(1.f-x, 0.f, x);
//	color = vec3(0.f, 1.f-x, x);

	return color;
}
