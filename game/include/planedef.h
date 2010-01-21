#ifndef CE_PLANEDEF_H
#define CE_PLANEDEF_H

#include "vec3def.h"
#include "planefwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

struct plane {
	vec3 n;
	float d;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_PLANEDEF_H */
