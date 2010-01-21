#include "vec3.h"
#include "plane.h"

plane* plane_init(float a, float b, float c, float d, plane* r)
{
	vec3_init(a, b, c, &r->n);
	r->d = d;

	return r;
}

plane* plane_init_tri(const vec3* a, const vec3* b, const vec3* c, plane* r)
{
	vec3 e1, e2;

	vec3_sub(b, a, &e1);
	vec3_sub(c, a, &e2);

	vec3_cross(&e1, &e2, &r->n);
	vec3_normalise(&r->n, &r->n);

	r->d = -vec3_dot(&r->n, a);

	return r;
}

plane* plane_normalise(const plane* a, plane* r)
{
	const float s = 1.0f / vec3_abs(&a->n);

	vec3_scale(&a->n, s, &r->n);
	r->d = a->d * s;

	return r;
}

float plane_dist(const plane* a, const vec3* b)
{
	return vec3_dot(&a->n, b) + a->d;
}
