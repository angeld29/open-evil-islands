#include <math.h>

#include "cemath.h"

const float PI = 3.14159265f;
const float PI2 = 6.28318531f;
const float PI_DIV_2 = 1.57079633f;
const float PI_DIV_4 = 0.78539816f;
const float PI_INV = 0.31830989f;

const float EPS_E4 = 1e-4f;
const float EPS_E5 = 1e-5f;
const float EPS_E6 = 1e-6f;

static const float DEG2RAD = 0.01745329f;
static const float RAD2DEG = 57.2957795f;

const float VECTOR2_ZERO[2] = { 0.0f, 0.0f };
const float VECTOR2_UNIT_X[2] = { 1.0f, 0.0f };
const float VECTOR2_UNIT_Y[2] = { 0.0f, 1.0f };
const float VECTOR2_UNIT_SCALE[2] = { 1.0f, 1.0f };
const float VECTOR2_NEG_UNIT_X[2] = { -1.0f, 0.0f };
const float VECTOR2_NEG_UNIT_Y[2] = { 0.0f, -1.0f };
const float VECTOR2_NEG_UNIT_SCALE[2] = { -1.0f, -1.0f };

const float VECTOR3_ZERO[3] = { 0.0f, 0.0f, 0.0f };
const float VECTOR3_UNIT_X[3] = { 1.0f, 0.0f, 0.0f };
const float VECTOR3_UNIT_Y[3] = { 0.0f, 1.0f, 0.0f };
const float VECTOR3_UNIT_Z[3] = { 0.0f, 0.0f, 1.0f };
const float VECTOR3_UNIT_SCALE[3] = { 1.0f, 1.0f, 1.0f };
const float VECTOR3_NEG_UNIT_X[3] = { -1.0f, 0.0f, 0.0f };
const float VECTOR3_NEG_UNIT_Y[3] = { 0.0f, -1.0f, 0.0f };
const float VECTOR3_NEG_UNIT_Z[3] = { 0.0f, 0.0f, -1.0f };
const float VECTOR3_NEG_UNIT_SCALE[3] = { -1.0f, -1.0f, -1.0f };

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
static float reldif(float a, float b)
{
	float c = fmaxf(fabsf(a), fabsf(b));
	return 0.0f == c ? 0.0f : fabsf(a - b) / c;
}

bool fisequal(float a, float b, float tolerance)
{
	return a == b || fabsf(a - b) <= tolerance || reldif(a, b) <= tolerance;
}

bool fiszero(float a, float tolerance)
{
	return fisequal(a, 0.0f, tolerance);
}

void fswap(float* a, float* b)
{
	float t = *a;
	*a = *b;
	*b = t;
}

float fclamp(float v, float a, float b)
{
	return v < a ? a : (v > b ? b : v);
}

float flerp(float u, float a, float b)
{
	return a + u * (b - a);
}

float deg2rad(float angle)
{
	return DEG2RAD * angle;
}

float rad2deg(float angle)
{
	return RAD2DEG * angle;
}

float* vector2_zero(float* r)
{
	r[0] = 0.0f;
	r[1] = 0.0f;
	return r;
}

float* vector2_unit_x(float* r)
{
	r[0] = 1.0f;
	r[1] = 0.0f;
	return r;
}

float* vector2_unit_y(float* r)
{
	r[0] = 0.0f;
	r[1] = 1.0f;
	return r;
}

float* vector2_unit_scale(float* r)
{
	r[0] = 1.0f;
	r[1] = 1.0f;
	return r;
}

float* vector2_neg_unit_x(float* r)
{
	r[0] = -1.0f;
	r[1] = 0.0f;
	return r;
}

float* vector2_neg_unit_y(float* r)
{
	r[0] = 0.0f;
	r[1] = -1.0f;
	return r;
}

float* vector2_neg_unit_scale(float* r)
{
	r[0] = -1.0f;
	r[1] = -1.0f;
	return r;
}

float* vector3_zero(float* r)
{
	r[0] = 0.0f;
	r[1] = 0.0f;
	r[2] = 0.0f;
	return r;
}

float* vector3_unit_x(float* r)
{
	r[0] = 1.0f;
	r[1] = 0.0f;
	r[2] = 0.0f;
	return r;
}

float* vector3_unit_y(float* r)
{
	r[0] = 0.0f;
	r[1] = 1.0f;
	r[2] = 0.0f;
	return r;
}

float* vector3_unit_z(float* r)
{
	r[0] = 0.0f;
	r[1] = 0.0f;
	r[2] = 1.0f;
	return r;
}

float* vector3_unit_scale(float* r)
{
	r[0] = 1.0f;
	r[1] = 1.0f;
	r[2] = 1.0f;
	return r;
}

float* vector3_neg_unit_x(float* r)
{
	r[0] = -1.0f;
	r[1] = 0.0f;
	r[2] = 0.0f;
	return r;
}

float* vector3_neg_unit_y(float* r)
{
	r[0] = 0.0f;
	r[1] = -1.0f;
	r[2] = 0.0f;
	return r;
}

float* vector3_neg_unit_z(float* r)
{
	r[0] = 0.0f;
	r[1] = 0.0f;
	r[2] = -1.0f;
	return r;
}

float* vector3_neg_unit_scale(float* r)
{
	r[0] = -1.0f;
	r[1] = -1.0f;
	r[2] = -1.0f;
	return r;
}

float* vector3_init(float x, float y, float z, float* r)
{
	r[0] = x;
	r[1] = y;
	r[2] = z;
	return r;
}

float* vector3_copy(const float* a, float* r)
{
	r[0] = a[0];
	r[1] = a[1];
	r[2] = a[2];
	return r;
}

float* vector3_neg(const float* a, float* r)
{
	r[0] = -a[0];
	r[1] = -a[1];
	r[2] = -a[2];
	return r;
}

float* vector3_add(const float* a, const float* b, float* r)
{
	r[0] = a[0] + b[0];
	r[1] = a[1] + b[1];
	r[2] = a[2] + b[2];
	return r;
}

float* vector3_sub(const float* a, const float* b, float* r)
{
	r[0] = a[0] - b[0];
	r[1] = a[1] - b[1];
	r[2] = a[2] - b[2];
	return r;
}

float* vector3_mul(const float* a, const float* b, float* r)
{
	r[0] = a[0] * b[0];
	r[1] = a[1] * b[1];
	r[2] = a[2] * b[2];
	return r;
}

float* vector3_div(const float* a, const float* b, float* r)
{
	r[0] = a[0] / b[0];
	r[1] = a[1] / b[1];
	r[2] = a[2] / b[2];
	return r;
}

float* vector3_scale(const float* a, float s, float* r)
{
	r[0] = a[0] * s;
	r[1] = a[1] * s;
	r[2] = a[2] * s;
	return r;
}

float vector3_abs(const float* a)
{
	return sqrtf(vector3_abs2(a));
}

float vector3_abs2(const float* a)
{
	return a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
}

float vector3_dist(const float* a, const float* b)
{
	float r[3];
	return vector3_abs(vector3_sub(a, b, r));
}

float vector3_dist2(const float* a, const float* b)
{
	float r[3];
	return vector3_abs2(vector3_sub(a, b, r));
}

float* vector3_normalise(const float* a, float* r)
{
	float s = 1.0f / vector3_abs(a);
	r[0] = a[0] * s;
	r[1] = a[1] * s;
	r[2] = a[2] * s;
	return r;
}

float vector3_dot(const float* a, const float* b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float* vector3_cross(const float* a, const float* b, float* restrict r)
{
	r[0] = a[1] * b[2] - a[2] * b[1];
	r[1] = a[2] * b[0] - a[0] * b[2];
	r[2] = a[0] * b[1] - a[1] * b[0];
	return r;
}

float* vector3_mid(const float* a, const float* b, float* r)
{
	r[0] = 0.5f * (a[0] + b[0]);
	r[1] = 0.5f * (a[1] + b[1]);
	r[2] = 0.5f * (a[2] + b[2]);
	return r;
}

float* vector3_rot(const float* a, const float* q, float* r)
{
	float qv[3] = { q[1], q[2], q[3] }, uv[3], uuv[3];
	vector3_cross(qv, a, uv);
	vector3_cross(qv, uv, uuv);
	vector3_scale(uv, 2.0f * q[0], uv);
	vector3_scale(uuv, 2.0f, uuv);
	return vector3_add(vector3_add(a, uv, r), uuv, r);
}

float* quaternion_zero(float* r)
{
	r[0] = 0.0f;
	r[1] = 0.0f;
	r[2] = 0.0f;
	r[3] = 0.0f;
	return r;
}

float* quaternion_identity(float* r)
{
	r[0] = 1.0f;
	r[1] = 0.0f;
	r[2] = 0.0f;
	r[3] = 0.0f;
	return r;
}

float* quaternion_init(float w, float x, float y, float z, float* r)
{
	r[0] = w;
	r[1] = x;
	r[2] = y;
	r[3] = z;
	return r;
}

float* quaternion_copy(const float* a, float* r)
{
	r[0] = a[0];
	r[1] = a[1];
	r[2] = a[2];
	r[3] = a[3];
	return r;
}

float* quaternion_polar(float theta, const float* axis, float* r)
{
	float t = 0.5f * theta;
	float s = sinf(t);
	r[0] = cosf(t);
	r[1] = s * axis[0];
	r[2] = s * axis[1];
	r[3] = s * axis[2];
	return r;
}

float* quaternion_neg(const float* a, float* r)
{
	r[0] = -a[0];
	r[1] = -a[1];
	r[2] = -a[2];
	r[3] = -a[3];
	return r;
}

float* quaternion_conj(const float* a, float* r)
{
	r[0] = a[0];
	r[1] = -a[1];
	r[2] = -a[2];
	r[3] = -a[3];
	return r;
}

float* quaternion_add(const float* a, const float* b, float* r)
{
	r[0] = a[0] + b[0];
	r[1] = a[1] + b[1];
	r[2] = a[2] + b[2];
	r[3] = a[3] + b[3];
	return r;
}

float* quaternion_sub(const float* a, const float* b, float* r)
{
	r[0] = a[0] - b[0];
	r[1] = a[1] - b[1];
	r[2] = a[2] - b[2];
	r[3] = a[3] - b[3];
	return r;
}

float* quaternion_mul(const float* a, const float* b, float* restrict r)
{
	r[0] = a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3];
	r[1] = a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2];
	r[2] = a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1];
	r[3] = a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0];
	return r;
}

float quaternion_abs(const float* a)
{
	return sqrtf(quaternion_abs2(a));
}

float quaternion_abs2(const float* a)
{
	return a[0] * a[0] + a[1] * a[1] + a[2] * a[2] + a[3] * a[3];
}

float quaternion_arg(const float* a)
{
	float l = quaternion_abs(a);
	return 0.0f == l ? 0.0f : acosf(a[0] / l);
}

float* quaternion_normalise(const float* a, float* r)
{
	float s = 1.0f / quaternion_abs(a);
	r[0] = a[0] * s;
	r[1] = a[1] * s;
	r[2] = a[2] * s;
	r[3] = a[3] * s;
	return r;
}

float* quaternion_inverse(const float* a, float* r)
{
	float s = 1.0f / quaternion_abs2(a);
	r[0] = a[0] * s;
	r[1] = a[1] * -s;
	r[2] = a[2] * -s;
	r[3] = a[3] * -s;
	return r;
}

float quaternion_dot(const float* a, const float* b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

float* matrix3_zero(float* r)
{
	r[0] = 0.0f; r[1] = 0.0f; r[2] = 0.0f;
	r[3] = 0.0f; r[4] = 0.0f; r[5] = 0.0f;
	r[6] = 0.0f; r[7] = 0.0f; r[8] = 0.0f;
	return r;
}

float* matrix3_identity(float* r)
{
	r[0] = 1.0f; r[1] = 0.0f; r[2] = 0.0f;
	r[3] = 0.0f; r[4] = 1.0f; r[5] = 0.0f;
	r[6] = 0.0f; r[7] = 0.0f; r[8] = 1.0f;
	return r;
}

float* matrix4_zero(float* r)
{
	r[0]  = 0.0f; r[1]  = 0.0f; r[2]  = 0.0f; r[3]  = 0.0f;
	r[4]  = 0.0f; r[5]  = 0.0f; r[6]  = 0.0f; r[7]  = 0.0f;
	r[8]  = 0.0f; r[9]  = 0.0f; r[10] = 0.0f; r[11] = 0.0f;
	r[12] = 0.0f; r[13] = 0.0f; r[14] = 0.0f; r[15] = 0.0f;
	return r;
}

float* matrix4_identity(float* r)
{
	r[0]  = 1.0f; r[1]  = 0.0f; r[2]  = 0.0f; r[3]  = 0.0f;
	r[4]  = 0.0f; r[5]  = 1.0f; r[6]  = 0.0f; r[7]  = 0.0f;
	r[8]  = 0.0f; r[9]  = 0.0f; r[10] = 1.0f; r[11] = 0.0f;
	r[12] = 0.0f; r[13] = 0.0f; r[14] = 0.0f; r[15] = 1.0f;
	return r;
}
