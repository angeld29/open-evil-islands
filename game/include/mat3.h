#ifndef CE_MAT3_H
#define CE_MAT3_H

#include "mat3def.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const mat3 MAT3_ZERO;
extern const mat3 MAT3_IDENTITY;

extern mat3* mat3_zero(mat3* r);
extern mat3* mat3_identity(mat3* r);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MAT3_H */
