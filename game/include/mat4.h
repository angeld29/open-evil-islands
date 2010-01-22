#ifndef CE_MAT4_H
#define CE_MAT4_H

#include "mat4def.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const mat4 MAT4_ZERO;
extern const mat4 MAT4_IDENTITY;

extern mat4* mat4_zero(mat4* r);
extern mat4* mat4_identity(mat4* r);

extern mat4* mat4_mul(const mat4* a, const mat4* b, mat4* restrict r);
extern mat4* mat4_inverse(const mat4* a, mat4* r);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MAT4_H */
