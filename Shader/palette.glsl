vec3 palette(float x)
{
	const float _palette_pi_ = 3.141592653589793238462643383279502884197169399375105820974f;
	x = clamp(x, 0.f, 1.f);

//	return vec3(x);                   // black->white
//	return vec3(x, 0.f, 0.f);         // black->red
//	return vec3(x*.8f+.1f, .1f, .1f); // dark red-grey->matt red
//	return vec3(0.f, x, 0.f);         // black->green
//	return vec3(.1f, x*.8f+.1f, .1f); // dark green-grey->matt green
//	return vec3(0.f, 0.f, x);         // black->blue
//	return vec3(.1f, .1f, x*.8f+.1f); // dark blue-grey->matt blue
	return vec3(sqrt(x), pow(x,3.f), clamp(sin(2.f * _palette_pi_ * x), 0.f, 1.f));
}
