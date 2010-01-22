#include <stdbool.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "cealloc.h"
#include "vec3.h"
#include "quat.h"
#include "mat4.h"
#include "camera.h"

struct camera {
	float fov;
	float aspect;
	float near;
	float far;
	vec3 eye;
	quat look;
	mat4 view;
	bool proj_changed;
	bool eye_changed;
	bool look_changed;
};

static void update_rotation(const quat* look, mat4* view)
{
	float tx  = 2.0f * look->x;
	float ty  = 2.0f * look->y;
	float tz  = 2.0f * look->z;
	float twx = tx * look->w;
	float twy = ty * look->w;
	float twz = tz * look->w;
	float txx = tx * look->x;
	float txy = ty * look->x;
	float txz = tz * look->x;
	float tyy = ty * look->y;
	float tyz = tz * look->y;
	float tzz = tz * look->z;

	view->m[0] = 1.0f - (tyy + tzz);
	view->m[1] = txy + twz;
	view->m[2] = txz - twy;

	view->m[4] = txy - twz;
	view->m[5] = 1.0f - (txx + tzz);
	view->m[6] = tyz + twx;

	view->m[8] = txz + twy;
	view->m[9] = tyz - twx;
	view->m[10] = 1.0f - (txx + tyy);
}

static void update_translation(const vec3* eye, mat4* view)
{
	view->m[12] =
		-view->m[0] * eye->x - view->m[4] * eye->y - view->m[8] * eye->z;
	view->m[13] =
		-view->m[1] * eye->x - view->m[5] * eye->y - view->m[9] * eye->z;
	view->m[14] =
		-view->m[2] * eye->x - view->m[6] * eye->y - view->m[10] * eye->z;
}

camera* camera_open(void)
{
	camera* cam = cealloc(sizeof(camera));
	if (NULL == cam) {
		return NULL;
	}
	cam->fov = 60.0f;
	cam->aspect = 1.0f;
	cam->near = 1.0f;
	cam->far = 500.0f;
	vec3_zero(&cam->eye);
	quat_identity(&cam->look);
	mat4_identity(&cam->view);
	cam->proj_changed = true;
	cam->eye_changed = true;
	cam->look_changed = true;
	return cam;
}

void camera_close(camera* cam)
{
	cefree(cam, sizeof(camera));
}

float camera_get_fov(camera* cam)
{
	return cam->fov;
}

float camera_get_aspect(camera* cam)
{
	return cam->aspect;
}

float camera_get_near(camera* cam)
{
	return cam->near;
}

float camera_get_far(camera* cam)
{
	return cam->far;
}

vec3* camera_get_eye(vec3* eye, camera* cam)
{
	return vec3_copy(&cam->eye, eye);
}

vec3* camera_get_forward(vec3* forward, camera* cam)
{
	quat q;
	return vec3_rot(&VEC3_NEG_UNIT_Z, quat_conj(&cam->look, &q), forward);
}

vec3* camera_get_up(vec3* up, camera* cam)
{
	quat q;
	return vec3_rot(&VEC3_UNIT_Y, quat_conj(&cam->look, &q), up);
}

vec3* camera_get_right(vec3* right, camera* cam)
{
	quat q;
	return vec3_rot(&VEC3_UNIT_X, quat_conj(&cam->look, &q), right);
}

void camera_set_fov(float fov, camera* cam)
{
	cam->fov = fov;
	cam->proj_changed = true;
}

void camera_set_aspect(int width, int height, camera* cam)
{
	cam->aspect = (float)width / height;
	cam->proj_changed = true;
}

void camera_set_near(float near, camera* cam)
{
	cam->near = near;
	cam->proj_changed = true;
}

void camera_set_far(float far, camera* cam)
{
	cam->far = far;
	cam->proj_changed = true;
}

void camera_set_eye(const vec3* eye, camera* cam)
{
	vec3_copy(eye, &cam->eye);
	cam->eye_changed = true;
}

void camera_set_look(const quat* look, camera* cam)
{
	quat_copy(look, &cam->look);
	cam->look_changed = true;
}

void camera_move(float offsetx, float offsetz, camera* cam)
{
	vec3 forward, right;

	camera_get_forward(&forward, cam);
	camera_get_right(&right, cam);

	// Ignore pitch difference angle.
	forward.y = 0.0f;
	right.y = 0.0f;

	vec3_normalise(&forward, &forward);
	vec3_normalise(&right, &right);

	vec3_scale(&forward, offsetz, &forward);
	vec3_scale(&right, offsetx, &right);

	vec3_add(&cam->eye, &forward, &cam->eye);
	vec3_add(&cam->eye, &right, &cam->eye);

	cam->eye_changed = true;
}

void camera_zoom(float offset, camera* cam)
{
	vec3 forward;
	camera_get_forward(&forward, cam);
	vec3_scale(&forward, offset, &forward);
	vec3_add(&cam->eye, &forward, &cam->eye);
	cam->eye_changed = true;
}

void camera_yaw_pitch(float psi, float theta, camera* cam)
{
	vec3 y;
	quat q, t;
	vec3_rot(&VEC3_UNIT_Y, &cam->look, &y);
	quat_mul(quat_init_polar(psi, &y, &q), &cam->look, &t);
	quat_mul(quat_init_polar(theta, &VEC3_UNIT_X, &q), &t, &cam->look);
	cam->look_changed = true;
}

void camera_setup(camera* cam)
{
	if (cam->proj_changed) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(cam->fov, cam->aspect, cam->near, cam->far);
		glMatrixMode(GL_MODELVIEW);
	}

	if (cam->look_changed) {
		update_rotation(&cam->look, &cam->view);
		update_translation(&cam->eye, &cam->view);
		glLoadMatrixf(cam->view.m);
		cam->look_changed = false;
		cam->eye_changed = false;
	}

	if (cam->eye_changed) {
		update_translation(&cam->eye, &cam->view);
		glLoadMatrixf(cam->view.m);
		cam->eye_changed = false;
	}
}
