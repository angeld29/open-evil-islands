#include "mat4.h"

const mat4 MAT4_ZERO = {
	.m = {
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	}
};

const mat4 MAT4_IDENTITY = {
	.m = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	}
};

mat4* mat4_zero(mat4* r)
{
	r->m[0]  = 0.0f; r->m[1]  = 0.0f; r->m[2]  = 0.0f; r->m[3]  = 0.0f;
	r->m[4]  = 0.0f; r->m[5]  = 0.0f; r->m[6]  = 0.0f; r->m[7]  = 0.0f;
	r->m[8]  = 0.0f; r->m[9]  = 0.0f; r->m[10] = 0.0f; r->m[11] = 0.0f;
	r->m[12] = 0.0f; r->m[13] = 0.0f; r->m[14] = 0.0f; r->m[15] = 0.0f;

	return r;
}

mat4* mat4_identity(mat4* r)
{
	r->m[0]  = 1.0f; r->m[1]  = 0.0f; r->m[2]  = 0.0f; r->m[3]  = 0.0f;
	r->m[4]  = 0.0f; r->m[5]  = 1.0f; r->m[6]  = 0.0f; r->m[7]  = 0.0f;
	r->m[8]  = 0.0f; r->m[9]  = 0.0f; r->m[10] = 1.0f; r->m[11] = 0.0f;
	r->m[12] = 0.0f; r->m[13] = 0.0f; r->m[14] = 0.0f; r->m[15] = 1.0f;

	return r;
}

mat4* mat4_mul(const mat4* a, const mat4* b, mat4* restrict r)
{
	r->m[0] = a->m[0] * b->m[0] + a->m[1] * b->m[4] +
				a->m[2] * b->m[8] + a->m[3] * b->m[12];
	r->m[1] = a->m[0] * b->m[1] + a->m[1] * b->m[5] +
				a->m[2] * b->m[9] + a->m[3] * b->m[13];
	r->m[2] = a->m[0] * b->m[2] + a->m[1] * b->m[6] +
				a->m[2] * b->m[10] + a->m[3] * b->m[14];
	r->m[3] = a->m[0] * b->m[3] + a->m[1] * b->m[7] +
				a->m[2] * b->m[11] + a->m[3] * b->m[15];
	r->m[4] = a->m[4] * b->m[0] + a->m[5] * b->m[4] +
				a->m[6] * b->m[8] + a->m[7] * b->m[12];
	r->m[5] = a->m[4] * b->m[1] + a->m[5] * b->m[5] +
				a->m[6] * b->m[9] + a->m[7] * b->m[13];
	r->m[6] = a->m[4] * b->m[2] + a->m[5] * b->m[6] +
				a->m[6] * b->m[10] + a->m[7] * b->m[14];
	r->m[7] = a->m[4] * b->m[3] + a->m[5] * b->m[7] +
				a->m[6] * b->m[11] + a->m[7] * b->m[15];
	r->m[8] = a->m[8] * b->m[0] + a->m[9] * b->m[4] +
				a->m[10] * b->m[8] + a->m[11] * b->m[12];
	r->m[9] = a->m[8] * b->m[1] + a->m[9] * b->m[5] +
				a->m[10] * b->m[9] + a->m[11] * b->m[13];
	r->m[10] = a->m[8] * b->m[2] + a->m[9] * b->m[6] +
				a->m[10] * b->m[10] + a->m[11] * b->m[14];
	r->m[11] = a->m[8] * b->m[3] + a->m[9] * b->m[7] +
				a->m[10] * b->m[11] + a->m[11] * b->m[15];
	r->m[12] = a->m[12] * b->m[0] + a->m[13] * b->m[4] +
				a->m[14] * b->m[8] + a->m[15] * b->m[12];
	r->m[13] = a->m[12] * b->m[1] + a->m[13] * b->m[5] +
				a->m[14] * b->m[9] + a->m[15] * b->m[13];
	r->m[14] = a->m[12] * b->m[2] + a->m[13] * b->m[6] +
				a->m[14] * b->m[10] + a->m[15] * b->m[14];
	r->m[15] = a->m[12] * b->m[3] + a->m[13] * b->m[7] +
				a->m[14] * b->m[11] + a->m[15] * b->m[15];

	return r;
}

mat4* mat4_inverse(const mat4* a, mat4* r)
{
	float m00 = a->m[0],  m01 = a->m[1],  m02 = a->m[2],  m03 = a->m[3];
	float m10 = a->m[4],  m11 = a->m[5],  m12 = a->m[6],  m13 = a->m[7];
	float m20 = a->m[8],  m21 = a->m[9],  m22 = a->m[10], m23 = a->m[11];
	float m30 = a->m[12], m31 = a->m[13], m32 = a->m[14], m33 = a->m[15];

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

	r->m[0]  = d00; r->m[1]  = d01; r->m[2]  = d02; r->m[3]  = d03;
	r->m[4]  = d10; r->m[5]  = d11; r->m[6]  = d12; r->m[7]  = d13;
	r->m[8]  = d20; r->m[9]  = d21; r->m[10] = d22; r->m[11] = d23;
	r->m[12] = d30; r->m[13] = d31; r->m[14] = d32; r->m[15] = d33;

	return r;
}
