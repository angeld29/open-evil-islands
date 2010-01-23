#ifndef CE_AABBDEF_H
#define CE_AABBDEF_H

#include "vec3def.h"
#include "aabbfwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

struct aabb {
	vec3 min, max;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_AABBDEF_H */
