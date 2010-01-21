#ifndef CE_VEC2_H
#define CE_VEC2_H

#include "vec2def.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const vec2 VEC2_ZERO;
extern const vec2 VEC2_UNIT_X;
extern const vec2 VEC2_UNIT_Y;
extern const vec2 VEC2_UNIT_SCALE;
extern const vec2 VEC2_NEG_UNIT_X;
extern const vec2 VEC2_NEG_UNIT_Y;
extern const vec2 VEC2_NEG_UNIT_SCALE;

extern vec2* vec2_zero(vec2* r);
extern vec2* vec2_unit_x(vec2* r);
extern vec2* vec2_unit_y(vec2* r);
extern vec2* vec2_unit_scale(vec2* r);
extern vec2* vec2_neg_unit_x(vec2* r);
extern vec2* vec2_neg_unit_y(vec2* r);
extern vec2* vec2_neg_unit_scale(vec2* r);

extern vec2* vec2_init(float x, float y, vec2* r);
extern vec2* vec2_copy(const vec2* a, vec2* r);

extern vec2* vec2_neg(const vec2* a, vec2* r);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VEC2_H */
