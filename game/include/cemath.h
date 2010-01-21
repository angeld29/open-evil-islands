#ifndef CE_CEMATH_H
#define CE_CEMATH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const float PI;
extern const float PI2;
extern const float PI_DIV_2;
extern const float PI_DIV_4;
extern const float PI_INV;

extern const float EPS_E4;
extern const float EPS_E5;
extern const float EPS_E6;

extern bool fisequal(float a, float b, float tolerance);
extern bool fiszero(float a, float tolerance);

extern void fswap(float* a, float* b);
extern float fclamp(float v, float a, float b);
extern float flerp(float u, float a, float b);

extern float deg2rad(float d);
extern float rad2deg(float r);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CEMATH_H */
