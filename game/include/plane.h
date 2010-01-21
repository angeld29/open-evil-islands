#ifndef CE_PLANE_H
#define CE_PLANE_H

#include "vec3fwd.h"
#include "planedef.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern plane* plane_init(float a, float b, float c, float d, plane* r);
extern plane* plane_init_tri(const vec3* a, const vec3* b,
								const vec3* c, plane* r);

extern plane* plane_normalise(const plane* a, plane* r);

extern float plane_dist(const plane* a, const vec3* b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_PLANE_H */
