#include "mat3.h"

const mat3 MAT3_ZERO = {
	.m = {
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f
	}
};

const mat3 MAT3_IDENTITY = {
	.m = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	}
};

mat3* mat3_zero(mat3* r)
{
	r->m[0] = 0.0f; r->m[1] = 0.0f; r->m[2] = 0.0f;
	r->m[3] = 0.0f; r->m[4] = 0.0f; r->m[5] = 0.0f;
	r->m[6] = 0.0f; r->m[7] = 0.0f; r->m[8] = 0.0f;
	return r;
}

mat3* mat3_identity(mat3* r)
{
	r->m[0] = 1.0f; r->m[1] = 0.0f; r->m[2] = 0.0f;
	r->m[3] = 0.0f; r->m[4] = 1.0f; r->m[5] = 0.0f;
	r->m[6] = 0.0f; r->m[7] = 0.0f; r->m[8] = 1.0f;
	return r;
}
