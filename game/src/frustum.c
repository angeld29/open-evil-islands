#include <stddef.h>
#include <math.h>

#include "cemath.h"
#include "frustum.h"

static float* get_box_vertex_positive(const float* normal, const float* pmin,
												const float* pmax, float* p)
{
	p[0] = normal[0] > 0.0f ? pmax[0] : pmin[0];
	p[1] = normal[1] > 0.0f ? pmax[1] : pmin[1];
	p[2] = normal[2] > 0.0f ? pmax[2] : pmin[2];
	return p;
}

frustum* frustum_init(float fov, float aspect, float near, float far,
						const float* eye, const float* forward,
						const float* right, const float* up, frustum* f)
{
	float tang = tanf(0.5f * deg2rad(fov));
	float nh = tang * near;
	float nw = nh * aspect;
	float fh = tang * far;
	float fw = fh * aspect;

	float nc[3], fc[3], xw[3], yh[3];
	float ntl[3], ntr[3], nbl[3], nbr[3], ftl[3], ftr[3], fbl[3], fbr[3];

	vector3_add(eye, vector3_scale(forward, near, nc), nc);
	vector3_add(eye, vector3_scale(forward, far, fc), fc);

	vector3_scale(right, nw, xw);
	vector3_scale(up, nh, yh);

	vector3_sub(vector3_add(nc, yh, ntl), xw, ntl);
	vector3_add(vector3_add(nc, yh, ntr), xw, ntr);
	vector3_sub(vector3_sub(nc, yh, nbl), xw, nbl);
	vector3_add(vector3_sub(nc, yh, nbr), xw, nbr);

	vector3_scale(right, fw, xw);
	vector3_scale(up, fh, yh);

	vector3_sub(vector3_add(fc, yh, ftl), xw, ftl);
	vector3_add(vector3_add(fc, yh, ftr), xw, ftr);
	vector3_sub(vector3_sub(fc, yh, fbl), xw, fbl);
	vector3_add(vector3_sub(fc, yh, fbr), xw, fbr);

	plane_tri(ntr, ntl, ftl, f->p[FRUSTUM_PLANE_TOP]);
	plane_tri(nbl, nbr, fbr, f->p[FRUSTUM_PLANE_BOTTOM]);
	plane_tri(ntl, nbl, fbl, f->p[FRUSTUM_PLANE_LEFT]);
	plane_tri(nbr, ntr, fbr, f->p[FRUSTUM_PLANE_RIGHT]);
	plane_tri(ntl, ntr, nbr, f->p[FRUSTUM_PLANE_NEAR]);
	plane_tri(ftr, ftl, fbl, f->p[FRUSTUM_PLANE_FAR]);

	return f;
}

bool frustum_test_point(const float* p, const frustum* f)
{
	for (size_t i = 0; i < FRUSTUM_PLANE_COUNT; ++i) {
		if (plane_dist(f->p[i], p) < 0.0f) {
			return false;
		}
	}
	return true;
}

bool frustum_test_sphere(const float* p, float r, const frustum* f)
{
	for (size_t i = 0; i < FRUSTUM_PLANE_COUNT; ++i) {
		if (plane_dist(f->p[i], p) < -r) {
			return false;
		}
	}
	return true;
}

bool frustum_test_box(const float* pmin, const float* pmax, const frustum* f)
{
	float p[3];
	for (size_t i = 0; i < FRUSTUM_PLANE_COUNT; ++i) {
		if (plane_dist(f->p[i],
				get_box_vertex_positive(f->p[i], pmin, pmax, p)) < 0.0f) {
			return false;
		}
	}
	return true;
}
