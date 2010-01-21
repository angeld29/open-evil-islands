#ifndef CE_FRUSTUM_H
#define CE_FRUSTUM_H

#include <stdbool.h>

#include "vec3fwd.h"
#include "planedef.h"

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
	plane p[FRUSTUM_PLANE_COUNT];
} frustum;

extern frustum* frustum_init(float fov, float aspect, float near, float far,
							const vec3* eye, const vec3* forward,
							const vec3* right, const vec3* up, frustum* f);

extern bool frustum_test_point(const vec3* p, const frustum* f);
extern bool frustum_test_sphere(const vec3* p, float r, const frustum* f);
extern bool frustum_test_box(const vec3* pmin, const vec3* pmax,
												const frustum* f);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FRUSTUM_H */
