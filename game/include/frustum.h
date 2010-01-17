#ifndef CE_FRUSTUM_H
#define CE_FRUSTUM_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	FRUSTUM_PLANE_TOP,
	FRUSTUM_PLANE_BOTTOM,
	FRUSTUM_PLANE_LEFT,
	FRUSTUM_PLANE_RIGHT,
	FRUSTUM_PLANE_NEAR,
	FRUSTUM_PLANE_FAR,
	FRUSTUM_PLANE_COUNT
} frustum_plane;

typedef struct {
	float p[FRUSTUM_PLANE_COUNT][4];
} frustum;

extern frustum* frustum_init(float fov, float aspect, float near, float far,
							const float* eye, const float* forward,
							const float* right, const float* up, frustum* f);

extern bool frustum_test_point(const float* p, const frustum* f);
extern bool frustum_test_sphere(const float* p, float r, const frustum* f);
extern bool frustum_test_box(const float* pmin, const float* pmax,
												const frustum* f);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FRUSTUM_H */
