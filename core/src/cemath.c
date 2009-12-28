#include <math.h>

#include "cemath.h"

#define CE_PI 3.1415926f

const float PI = CE_PI;
const float HALF_PI = 0.5f * CE_PI;
const float TWO_PI = 2.0f * CE_PI;

const float DEG2RAD = CE_PI / 180.0f;
const float RAD2DEG = 180.0f / CE_PI;

const float VECTOR2_ZERO[2] = { 0.0f, 0.0f };
const float VECTOR2_UNIT_X[2] = { 1.0f, 0.0f };
const float VECTOR2_UNIT_Y[2] = { 0.0f, 1.0f };
const float VECTOR2_UNIT_SCALE[2] = { 1.0f, 1.0f };

const float VECTOR3_ZERO[3] = { 0.0f, 0.0f, 0.0f };
const float VECTOR3_UNIT_X[3] = { 1.0f, 0.0f, 0.0f };
const float VECTOR3_UNIT_Y[3] = { 0.0f, 1.0f, 0.0f };
const float VECTOR3_UNIT_Z[3] = { 0.0f, 0.0f, 1.0f };
const float VECTOR3_UNIT_SCALE[3] = { 1.0f, 1.0f, 1.0f };

const float QUATERNION_ZERO[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
const float QUATERNION_IDENTITY[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

const float MATRIX3_ZERO[9] = {
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
};

const float MATRIX3_IDENTITY[9] = {
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
};

const float MATRIX4_ZERO[16] = {
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
};

const float MATRIX4_IDENTITY[16] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

/**
 *  Based on http://www.c-faq.com/fp/fpequal.html.
*/
static float reldiff(float a, float b)
{
	float c = fmaxf(fabsf(a), fabsf(b));
	return 0.0f == c ? 0.0f : fabsf(a - b) / c;
}

bool fisequalf(float a, float b, float tolerance)
{
	return a == b || fabsf(a - b) <= tolerance || reldiff(a, b) <= tolerance;
}

float* vector2_zero(float r[2])
{
	r[0] = 0.0f;
	r[1] = 0.0f;
	return r;
}

float* vector2_unit_x(float r[2])
{
	r[0] = 1.0f;
	r[1] = 0.0f;
	return r;
}

float* vector2_unit_y(float r[2])
{
	r[0] = 0.0f;
	r[1] = 1.0f;
	return r;
}

float* vector2_unit_scale(float r[2])
{
	r[0] = 1.0f;
	r[1] = 1.0f;
	return r;
}

float* vector3_zero(float r[3])
{
	r[0] = 0.0f;
	r[1] = 0.0f;
	r[2] = 0.0f;
	return r;
}

float* vector3_unit_x(float r[3])
{
	r[0] = 1.0f;
	r[1] = 0.0f;
	r[2] = 0.0f;
	return r;
}

float* vector3_unit_y(float r[3])
{
	r[0] = 0.0f;
	r[1] = 1.0f;
	r[2] = 0.0f;
	return r;
}

float* vector3_unit_z(float r[3])
{
	r[0] = 0.0f;
	r[1] = 0.0f;
	r[2] = 1.0f;
	return r;
}

float* vector3_unit_scale(float r[3])
{
	r[0] = 1.0f;
	r[1] = 1.0f;
	r[2] = 1.0f;
	return r;
}

float* vector3_init(float x, float y, float z, float r[3])
{
	r[0] = x;
	r[1] = y;
	r[2] = z;
	return r;
}

float* vector3_copy(const float a[3], float r[3])
{
	r[0] = a[0];
	r[1] = a[1];
	r[2] = a[2];
	return r;
}

float* vector3_neg(const float a[3], float r[3])
{
	r[0] = -a[0];
	r[1] = -a[1];
	r[2] = -a[2];
	return r;
}

float* vector3_add(const float a[3], const float b[3], float r[3])
{
	r[0] = a[0] + b[0];
	r[1] = a[1] + b[1];
	r[2] = a[2] + b[2];
	return r;
}

float* vector3_sub(const float a[3], const float b[3], float r[3])
{
	r[0] = a[0] - b[0];
	r[1] = a[1] - b[1];
	r[2] = a[2] - b[2];
	return r;
}

float* vector3_mul(const float a[3], const float b[3], float r[3])
{
	r[0] = a[0] * b[0];
	r[1] = a[1] * b[1];
	r[2] = a[2] * b[2];
	return r;
}

float* vector3_div(const float a[3], const float b[3], float r[3])
{
	r[0] = a[0] / b[0];
	r[1] = a[1] / b[1];
	r[2] = a[2] / b[2];
	return r;
}

float* vector3_add_scalar(const float a[3], float s, float r[3])
{
	r[0] = a[0] + s;
	r[1] = a[1] + s;
	r[2] = a[2] + s;
	return r;
}

float* vector3_sub_scalar(const float a[3], float s, float r[3])
{
	r[0] = a[0] - s;
	r[1] = a[1] - s;
	r[2] = a[2] - s;
	return r;
}

float* vector3_mul_scalar(const float a[3], float s, float r[3])
{
	r[0] = a[0] * s;
	r[1] = a[1] * s;
	r[2] = a[2] * s;
	return r;
}

float* vector3_div_scalar(const float a[3], float s, float r[3])
{
	float i = 1.0f / s;
	r[0] = a[0] * i;
	r[1] = a[1] * i;
	r[2] = a[2] * i;
	return r;
}

float vector3_len(const float a[3])
{
	return sqrtf(vector3_len2(a));
}

float vector3_len2(const float a[3])
{
	return a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
}

float vector3_dist(const float a[3], const float b[3])
{
	float r[3];
	return vector3_len(vector3_sub(a, b, r));
}

float vector3_dist2(const float a[3], const float b[3])
{
	float r[3];
	return vector3_len2(vector3_sub(a, b, r));
}

float* vector3_normalise(const float a[3], float r[3])
{
	float i = 1.0f / vector3_len(a);
	r[0] = a[0] * i;
	r[1] = a[1] * i;
	r[2] = a[2] * i;
	return r;
}

float vector3_dot(const float a[3], const float b[3])
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float* vector3_cross(const float a[3], const float b[3], float r[restrict 3])
{
	r[0] = a[1] * b[2] - a[2] * b[1];
	r[1] = a[2] * b[0] - a[0] * b[2];
	r[2] = a[0] * b[1] - a[1] * b[0];
	return r;
}

float* vector3_mid(const float a[3], const float b[3], float r[3])
{
	r[0] = 0.5f * (a[0] + b[0]);
	r[1] = 0.5f * (a[1] + b[1]);
	r[2] = 0.5f * (a[2] + b[2]);
	return r;
}

float* vector3_rot(const float vec[3], const float quat[4], float r[3])
{
	float qvec[3] = { quat[1], quat[2], quat[3] }, uv[3], uuv[3];
	vector3_cross(qvec, vec, uv);
	vector3_cross(qvec, uv, uuv);
	vector3_mul_scalar(uv, 2.0f * quat[0], uv);
	vector3_mul_scalar(uuv, 2.0f, uuv);
	return vector3_add(vector3_add(vec, uv, r), uuv, r);
}

float* quaternion_zero(float r[4])
{
	r[0] = 0.0f;
	r[1] = 0.0f;
	r[2] = 0.0f;
	r[3] = 0.0f;
	return r;
}

float* quaternion_identity(float r[4])
{
	r[0] = 1.0f;
	r[1] = 0.0f;
	r[2] = 0.0f;
	r[3] = 0.0f;
	return r;
}

float* quaternion_init(float w, float x, float y, float z, float r[4])
{
	r[0] = w;
	r[1] = x;
	r[2] = y;
	r[3] = z;
	return r;
}

float* quaternion_copy(const float a[4], float r[4])
{
	r[0] = a[0];
	r[1] = a[1];
	r[2] = a[2];
	r[3] = a[3];
	return r;
}

float* quaternion_polar(float theta, const float axis[3], float r[4])
{
	float t = 0.5f * theta;
	float s = sinf(t);
	r[0] = cosf(t);
	r[1] = s * axis[0];
	r[2] = s * axis[1];
	r[3] = s * axis[2];
	return r;
}

float* quaternion_neg(const float a[4], float r[4])
{
	r[0] = -a[0];
	r[1] = -a[1];
	r[2] = -a[2];
	r[3] = -a[3];
	return r;
}

float* quaternion_conj(const float a[4], float r[4])
{
	r[0] = a[0];
	r[1] = -a[1];
	r[2] = -a[2];
	r[3] = -a[3];
	return r;
}

float* quaternion_add(const float a[4], const float b[4], float r[4])
{
	r[0] = a[0] + b[0];
	r[1] = a[1] + b[1];
	r[2] = a[2] + b[2];
	r[3] = a[3] + b[3];
	return r;
}

float* quaternion_sub(const float a[4], const float b[4], float r[4])
{
	r[0] = a[0] - b[0];
	r[1] = a[1] - b[1];
	r[2] = a[2] - b[2];
	r[3] = a[3] - b[3];
	return r;
}

float* quaternion_mul(const float a[4], const float b[4], float r[restrict 4])
{
	r[0] = a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3];
	r[1] = a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2];
	r[2] = a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1];
	r[3] = a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0];
	return r;
}

float* quaternion_mul_scalar(const float a[4], float s, float r[4])
{
	r[0] = a[0] * s;
	r[1] = a[1] * s;
	r[2] = a[2] * s;
	r[3] = a[3] * s;
	return r;
}

float quaternion_len(const float a[4])
{
	return sqrtf(quaternion_len2(a));
}

float quaternion_len2(const float a[4])
{
	return a[0] * a[0] + a[1] * a[1] + a[2] * a[2] + a[3] * a[3];
}

float quaternion_arg(const float a[4])
{
	float l = quaternion_len(a);
	return 0.0f == l ? 0.0f : acosf(a[0] / l);
}

float* quaternion_normalise(const float a[4], float r[4])
{
	float i = 1.0f / quaternion_len(a);
	r[0] = a[0] * i;
	r[1] = a[1] * i;
	r[2] = a[2] * i;
	r[3] = a[3] * i;
	return r;
}

float* quaternion_invert(const float a[4], float r[4])
{
	float i = 1.0f / quaternion_len2(a);
	r[0] = a[0] * i;
	r[1] = a[1] * -i;
	r[2] = a[2] * -i;
	r[3] = a[3] * -i;
	return r;
}

float quaternion_dot(const float a[4], const float b[4])
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

float* matrix3_zero(float r[9])
{
	r[0] = 0.0f; r[1] = 0.0f; r[2] = 0.0f;
	r[3] = 0.0f; r[4] = 0.0f; r[5] = 0.0f;
	r[6] = 0.0f; r[7] = 0.0f; r[8] = 0.0f;
	return r;
}

float* matrix3_identity(float r[9])
{
	r[0] = 1.0f; r[1] = 0.0f; r[2] = 0.0f;
	r[3] = 0.0f; r[4] = 1.0f; r[5] = 0.0f;
	r[6] = 0.0f; r[7] = 0.0f; r[8] = 1.0f;
	return r;
}

float* matrix4_zero(float r[16])
{
	r[0]  = 0.0f; r[1]  = 0.0f; r[2]  = 0.0f; r[3]  = 0.0f;
	r[4]  = 0.0f; r[5]  = 0.0f; r[6]  = 0.0f; r[7]  = 0.0f;
	r[8]  = 0.0f; r[9]  = 0.0f; r[10] = 0.0f; r[11] = 0.0f;
	r[12] = 0.0f; r[13] = 0.0f; r[14] = 0.0f; r[15] = 0.0f;
	return r;
}

float* matrix4_identity(float r[16])
{
	r[0]  = 1.0f; r[1]  = 0.0f; r[2]  = 0.0f; r[3]  = 0.0f;
	r[4]  = 0.0f; r[5]  = 1.0f; r[6]  = 0.0f; r[7]  = 0.0f;
	r[8]  = 0.0f; r[9]  = 0.0f; r[10] = 1.0f; r[11] = 0.0f;
	r[12] = 0.0f; r[13] = 0.0f; r[14] = 0.0f; r[15] = 1.0f;
	return r;
}
