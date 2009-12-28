#ifndef CE_CEMATH_H
#define CE_CEMATH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const float PI;
extern const float HALF_PI;
extern const float TWO_PI;

extern const float DEG2RAD;
extern const float RAD2DEG;

extern const float VECTOR2_ZERO[2];
extern const float VECTOR2_UNIT_X[2];
extern const float VECTOR2_UNIT_Y[2];
extern const float VECTOR2_UNIT_SCALE[2];

extern const float VECTOR3_ZERO[3];
extern const float VECTOR3_UNIT_X[3];
extern const float VECTOR3_UNIT_Y[3];
extern const float VECTOR3_UNIT_Z[3];
extern const float VECTOR3_UNIT_SCALE[3];

extern const float QUATERNION_ZERO[4];
extern const float QUATERNION_IDENTITY[4];

extern const float MATRIX3_ZERO[9];
extern const float MATRIX3_IDENTITY[9];

extern const float MATRIX4_ZERO[16];
extern const float MATRIX4_IDENTITY[16];

extern bool fisequalf(float a, float b, float tolerance);

static inline bool fiszerof(float a, float tolerance)
{
	return fisequalf(a, 0.0f, tolerance);
}

static inline float deg2rad(float angle)
{
	return angle * DEG2RAD;
}

static inline float rad2deg(float angle)
{
	return angle * RAD2DEG;
}

extern float* vector2_zero(float r[2]);
extern float* vector2_unit_x(float r[2]);
extern float* vector2_unit_y(float r[2]);
extern float* vector2_unit_scale(float r[2]);

extern float* vector3_zero(float r[3]);
extern float* vector3_unit_x(float r[3]);
extern float* vector3_unit_y(float r[3]);
extern float* vector3_unit_z(float r[3]);
extern float* vector3_unit_scale(float r[3]);

extern float* vector3_init(float x, float y, float z, float r[3]);
extern float* vector3_copy(const float a[3], float r[3]);

extern float* vector3_neg(const float a[3], float r[3]);

extern float* vector3_add(const float a[3], const float b[3], float r[3]);
extern float* vector3_sub(const float a[3], const float b[3], float r[3]);
extern float* vector3_mul(const float a[3], const float b[3], float r[3]);
extern float* vector3_div(const float a[3], const float b[3], float r[3]);

extern float* vector3_add_scalar(const float a[3], float s, float r[3]);
extern float* vector3_sub_scalar(const float a[3], float s, float r[3]);
extern float* vector3_mul_scalar(const float a[3], float s, float r[3]);
extern float* vector3_div_scalar(const float a[3], float s, float r[3]);

extern float vector3_len(const float a[3]);
extern float vector3_len2(const float a[3]);

extern float vector3_dist(const float a[3], const float b[3]);
extern float vector3_dist2(const float a[3], const float b[3]);

extern float* vector3_normalise(const float a[3], float r[3]);

extern float vector3_dot(const float a[3], const float b[3]);
extern float* vector3_cross(const float a[3],
							const float b[3], float r[restrict 3]);

extern float* vector3_mid(const float a[3], const float b[3], float r[3]);
extern float* vector3_rot(const float vec[3], const float quat[4], float r[3]);

extern float* quaternion_zero(float r[4]);
extern float* quaternion_identity(float r[4]);

extern float* quaternion_init(float w, float x, float y, float z, float r[4]);
extern float* quaternion_copy(const float a[4], float r[4]);
extern float* quaternion_polar(float theta, const float axis[3], float r[4]);

extern float* quaternion_neg(const float a[4], float r[4]);
extern float* quaternion_conj(const float a[4], float r[4]);

extern float* quaternion_add(const float a[4], const float b[4], float r[4]);
extern float* quaternion_sub(const float a[4], const float b[4], float r[4]);
extern float* quaternion_mul(const float a[4],
							const float b[4], float r[restrict 4]);

extern float* quaternion_mul_scalar(const float a[4], float s, float r[4]);

extern float quaternion_len(const float a[4]);
extern float quaternion_len2(const float a[4]);

extern float quaternion_arg(const float a[4]);

extern float* quaternion_normalise(const float a[4], float r[4]);
extern float* quaternion_invert(const float a[4], float r[4]);

extern float quaternion_dot(const float a[4], const float b[4]);

extern float* matrix3_zero(float r[9]);
extern float* matrix3_identity(float r[9]);

extern float* matrix4_zero(float r[16]);
extern float* matrix4_identity(float r[16]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CEMATH_H */
