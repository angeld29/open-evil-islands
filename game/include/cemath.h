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

extern const float VECTOR2_ZERO[2];
extern const float VECTOR2_UNIT_X[2];
extern const float VECTOR2_UNIT_Y[2];
extern const float VECTOR2_UNIT_SCALE[2];
extern const float VECTOR2_NEG_UNIT_X[2];
extern const float VECTOR2_NEG_UNIT_Y[2];
extern const float VECTOR2_NEG_UNIT_SCALE[2];

extern const float VECTOR3_ZERO[3];
extern const float VECTOR3_UNIT_X[3];
extern const float VECTOR3_UNIT_Y[3];
extern const float VECTOR3_UNIT_Z[3];
extern const float VECTOR3_UNIT_SCALE[3];
extern const float VECTOR3_NEG_UNIT_X[3];
extern const float VECTOR3_NEG_UNIT_Y[3];
extern const float VECTOR3_NEG_UNIT_Z[3];
extern const float VECTOR3_NEG_UNIT_SCALE[3];

extern const float QUATERNION_ZERO[4];
extern const float QUATERNION_IDENTITY[4];

extern const float MATRIX3_ZERO[9];
extern const float MATRIX3_IDENTITY[9];

extern const float MATRIX4_ZERO[16];
extern const float MATRIX4_IDENTITY[16];

extern bool fisequal(float a, float b, float tolerance);
extern bool fiszero(float a, float tolerance);

extern void fswap(float* a, float* b);
extern float fclamp(float v, float a, float b);
extern float flerp(float u, float a, float b);

extern float deg2rad(float angle);
extern float rad2deg(float angle);

extern float* vector2_zero(float* r);
extern float* vector2_unit_x(float* r);
extern float* vector2_unit_y(float* r);
extern float* vector2_unit_scale(float* r);
extern float* vector2_neg_unit_x(float* r);
extern float* vector2_neg_unit_y(float* r);
extern float* vector2_neg_unit_scale(float* r);

extern float* vector3_zero(float* r);
extern float* vector3_unit_x(float* r);
extern float* vector3_unit_y(float* r);
extern float* vector3_unit_z(float* r);
extern float* vector3_unit_scale(float* r);
extern float* vector3_neg_unit_x(float* r);
extern float* vector3_neg_unit_y(float* r);
extern float* vector3_neg_unit_z(float* r);
extern float* vector3_neg_unit_scale(float* r);

extern float* vector3_init(float x, float y, float z, float* r);
extern float* vector3_copy(const float* a, float* r);

extern float* vector3_neg(const float* a, float* r);

extern float* vector3_add(const float* a, const float* b, float* r);
extern float* vector3_sub(const float* a, const float* b, float* r);
extern float* vector3_mul(const float* a, const float* b, float* r);
extern float* vector3_div(const float* a, const float* b, float* r);

extern float* vector3_scale(const float* a, float s, float* r);

extern float vector3_abs(const float* a);
extern float vector3_abs2(const float* a);

extern float vector3_dist(const float* a, const float* b);
extern float vector3_dist2(const float* a, const float* b);

extern float* vector3_normalise(const float* a, float* r);

extern float vector3_dot(const float* a, const float* b);
extern float vector3_absdot(const float* a, const float* b);

extern float* vector3_cross(const float* a, const float* b, float* restrict r);

extern float* vector3_mid(const float* a, const float* b, float* r);
extern float* vector3_rot(const float* a, const float* q, float* r);

extern float* quaternion_zero(float* r);
extern float* quaternion_identity(float* r);

extern float* quaternion_init(float w, float x, float y, float z, float* r);
extern float* quaternion_copy(const float* a, float* r);
extern float* quaternion_polar(float theta, const float* axis, float* r);

extern float* quaternion_neg(const float* a, float* r);
extern float* quaternion_conj(const float* a, float* r);

extern float* quaternion_add(const float* a, const float* b, float* r);
extern float* quaternion_sub(const float* a, const float* b, float* r);
extern float* quaternion_mul(const float* a, const float* b, float* restrict r);

extern float quaternion_abs(const float* a);
extern float quaternion_abs2(const float* a);

extern float quaternion_arg(const float* a);

extern float* quaternion_normalise(const float* a, float* r);
extern float* quaternion_inverse(const float* a, float* r);

extern float quaternion_dot(const float* a, const float* b);

extern float* matrix3_zero(float* r);
extern float* matrix3_identity(float* r);

extern float* matrix4_zero(float* r);
extern float* matrix4_identity(float* r);

extern float* matrix4_mul(const float* a, const float* b, float* restrict r);
extern float* matrix4_inverse(const float* a, float* r);

extern float* plane_tri(const float* a, const float* b, const float* c, float* r);

extern float plane_dist(const float* p, const float* v);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CEMATH_H */
