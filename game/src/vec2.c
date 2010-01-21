#include "vec2.h"

const vec2 VEC2_ZERO = { .x = 0.0f, .y = 0.0f };
const vec2 VEC2_UNIT_X = { .x = 1.0f, .y = 0.0f };
const vec2 VEC2_UNIT_Y = { .x = 0.0f, .y = 1.0f };
const vec2 VEC2_UNIT_SCALE = { .x = 1.0f, .y = 1.0f };
const vec2 VEC2_NEG_UNIT_X = { .x = -1.0f, .y = 0.0f };
const vec2 VEC2_NEG_UNIT_Y = { .x = 0.0f, .y = -1.0f };
const vec2 VEC2_NEG_UNIT_SCALE = { .x = -1.0f, .y = -1.0f };

vec2* vec2_zero(vec2* r)
{
	r->x = 0.0f;
	r->y = 0.0f;
	return r;
}

vec2* vec2_unit_x(vec2* r)
{
	r->x = 1.0f;
	r->y = 0.0f;
	return r;
}

vec2* vec2_unit_y(vec2* r)
{
	r->x = 0.0f;
	r->y = 1.0f;
	return r;
}

vec2* vec2_unit_scale(vec2* r)
{
	r->x = 1.0f;
	r->y = 1.0f;
	return r;
}

vec2* vec2_neg_unit_x(vec2* r)
{
	r->x = -1.0f;
	r->y = 0.0f;
	return r;
}

vec2* vec2_neg_unit_y(vec2* r)
{
	r->x = 0.0f;
	r->y = -1.0f;
	return r;
}

vec2* vec2_neg_unit_scale(vec2* r)
{
	r->x = -1.0f;
	r->y = -1.0f;
	return r;
}

vec2* vec2_init(float x, float y, vec2* r)
{
	r->x = x;
	r->y = y;
	return r;
}

vec2* vec2_copy(const vec2* a, vec2* r)
{
	r->x = a->x;
	r->y = a->y;
	return r;
}

vec2* vec2_neg(const vec2* a, vec2* r)
{
	r->x = -a->x;
	r->y = -a->y;
	return r;
}
