#ifndef CE_MATH_H
#define CE_MATH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const float CEPI;
extern const float CEPI2;
extern const float CEPI_DIV_2;
extern const float CEPI_DIV_4;
extern const float CEPI_INV;

extern const float CEEPS_E4;
extern const float CEEPS_E5;
extern const float CEEPS_E6;

extern bool cefisequal(float a, float b, float tolerance);
extern bool cefiszero(float a, float tolerance);

extern void cefswap(float* a, float* b);
extern float cefclamp(float v, float a, float b);

extern float celerp(float u, float a, float b);

extern float cedeg2rad(float d);
extern float cerad2deg(float r);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MATH_H */
