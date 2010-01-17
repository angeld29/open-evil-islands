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

float vector3_absdot(const float* a, const float* b)
{
	return fabsf(a[0] * b[0]) + fabsf(a[1] * b[1]) + fabsf(a[2] * b[2]);
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

float* matrix4_mul(const float* a, const float* b, float* restrict r)
{
	r[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
	r[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
	r[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
	r[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];
	r[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
	r[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
	r[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
	r[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];
	r[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
	r[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
	r[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
	r[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];
	r[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
	r[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
	r[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
	r[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];
	return r;
}

float* matrix4_inverse(const float* a, float* r)
{
	float m00 = a[0],  m01 = a[1],  m02 = a[2],  m03 = a[3];
	float m10 = a[4],  m11 = a[5],  m12 = a[6],  m13 = a[7];
	float m20 = a[8],  m21 = a[9],  m22 = a[10], m23 = a[11];
	float m30 = a[12], m31 = a[13], m32 = a[14], m33 = a[15];

	float v0 = m20 * m31 - m21 * m30;
	float v1 = m20 * m32 - m22 * m30;
	float v2 = m20 * m33 - m23 * m30;
	float v3 = m21 * m32 - m22 * m31;
	float v4 = m21 * m33 - m23 * m31;
	float v5 = m22 * m33 - m23 * m32;

	float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
	float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
	float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
	float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

	float id = 1.0f / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

	float d00 = t00 * id;
	float d10 = t10 * id;
	float d20 = t20 * id;
	float d30 = t30 * id;

	float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * id;
	float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * id;
	float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * id;
	float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * id;

	v0 = m10 * m31 - m11 * m30;
	v1 = m10 * m32 - m12 * m30;
	v2 = m10 * m33 - m13 * m30;
	v3 = m11 * m32 - m12 * m31;
	v4 = m11 * m33 - m13 * m31;
	v5 = m12 * m33 - m13 * m32;

	float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * id;
	float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * id;
	float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * id;
	float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * id;

	v0 = m21 * m10 - m20 * m11;
	v1 = m22 * m10 - m20 * m12;
	v2 = m23 * m10 - m20 * m13;
	v3 = m22 * m11 - m21 * m12;
	v4 = m23 * m11 - m21 * m13;
	v5 = m23 * m12 - m22 * m13;

	float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * id;
	float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * id;
	float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * id;
	float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * id;

	r[0]  = d00; r[1]  = d01; r[2]  = d02; r[3]  = d03;
	r[4]  = d10; r[5]  = d11; r[6]  = d12; r[7]  = d13;
	r[8]  = d20; r[9]  = d21; r[10] = d22; r[11] = d23;
	r[12] = d30; r[13] = d31; r[14] = d32; r[15] = d33;
	return r;
}

float* plane_tri(const float* a, const float* b, const float* c, float* r)
{
	float e1[3], e2[3];

	vector3_sub(b, a, e1);
	vector3_sub(c, a, e2);

	vector3_cross(e1, e2, r);
	vector3_normalise(r, r);

	r[3] = -vector3_dot(r, a);

	return r;
}

float plane_dist(const float* p, const float* v)
{
	return vector3_dot(p, v) + p[3];
}
