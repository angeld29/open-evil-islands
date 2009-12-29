#include <stdlib.h>
#include <stdbool.h>

#include <GL/gl.h>

#include "cemath.h"
#include "camera.h"

struct camera {
	float eye[3];
	float look[4];
	float view[16];
	bool eye_changed;
	bool look_changed;
};

static void update_rotation(float look[4], float view[16])
{
	float tx  = 2.0f * look[1];
	float ty  = 2.0f * look[2];
	float tz  = 2.0f * look[3];
	float twx = tx * look[0];
	float twy = ty * look[0];
	float twz = tz * look[0];
	float txx = tx * look[1];
	float txy = ty * look[1];
	float txz = tz * look[1];
	float tyy = ty * look[2];
	float tyz = tz * look[2];
	float tzz = tz * look[3];

	view[0] = 1.0f - (tyy + tzz);
	view[1] = txy + twz;
	view[2] = txz - twy;

	view[4] = txy - twz;
	view[5] = 1.0f - (txx + tzz);
	view[6] = tyz + twx;

	view[8] = txz + twy;
	view[9] = tyz - twx;
	view[10] = 1.0f - (txx + tyy);
}

static void update_translation(float eye[3], float view[16])
{
	view[12] = -view[0] * eye[0] - view[4] * eye[1] - view[8] * eye[2];
	view[13] = -view[1] * eye[0] - view[5] * eye[1] - view[9] * eye[2];
	view[14] = -view[2] * eye[0] - view[6] * eye[1] - view[10] * eye[2];
}

camera* camera_new(void)
{
	camera* cam = malloc(sizeof(camera));
	if (NULL == cam) {
		return NULL;
	}
	vector3_zero(cam->eye);
	quaternion_identity(cam->look);
	cam->view[3] = 0.0f;
	cam->view[7] = 0.0f;
	cam->view[11] = 0.0f;
	cam->view[15] = 1.0f;
	cam->eye_changed = true;
	cam->look_changed = true;
	return cam;
}

void camera_delete(camera* cam)
{
	free(cam);
}

float* camera_eye(float eye[3], const camera* cam)
{
	return vector3_copy(cam->eye, eye);
}

float* camera_dir(float dir[3], const camera* cam)
{
	float q[4];
	return vector3_rot(VECTOR3_NEG_UNIT_Z,
		quaternion_inverse(cam->look, q), dir);
}

float* camera_up(float up[3], const camera* cam)
{
	float q[4];
	return vector3_rot(VECTOR3_UNIT_Y,
		quaternion_inverse(cam->look, q), up);
}

float* camera_right(float right[3], const camera* cam)
{
	float q[4];
	return vector3_rot(VECTOR3_UNIT_X,
		quaternion_inverse(cam->look, q), right);
}

void camera_set_eye(const float eye[3], camera* cam)
{
	vector3_copy(eye, cam->eye);
	cam->eye_changed = true;
}

void camera_set_look(const float look[4], camera* cam)
{
	quaternion_copy(look, cam->look);
	cam->look_changed = true;
}

void camera_move(const float offset[2], camera* cam)
{
	vector3_add(cam->eye, (float[]){ offset[0], 0.0f, offset[1] }, cam->eye);

	/*float t[3];
	vector3_add(cam->eye, vector3_rot((float[]){ offset[0], 0.0f, offset[1] },
		cam->look, t), cam->eye);*/

	cam->eye_changed = true;
}

void camera_zoom(float offset, camera* cam)
{
	float dir[3];
	camera_dir(dir, cam);
	vector3_mul_scalar(dir, offset, dir);
	vector3_add(cam->eye, dir, cam->eye);
	cam->eye_changed = true;
}

void camera_yaw_pitch(float psi, float theta, camera* cam)
{
	float y[3], q[4], t[4];
	vector3_rot(VECTOR3_UNIT_Y, cam->look, y);
	quaternion_mul(quaternion_polar(psi, y, q), cam->look, t);
	quaternion_mul(quaternion_polar(theta, VECTOR3_UNIT_X, q), t, cam->look);
	cam->look_changed = true;
}

void camera_setup(camera* cam)
{
	if (cam->look_changed) {
		update_rotation(cam->look, cam->view);
		update_translation(cam->eye, cam->view);
		cam->look_changed = false;
		cam->eye_changed = false;
	}
	if (cam->eye_changed) {
		update_translation(cam->eye, cam->view);
		cam->eye_changed = false;
	}
	glLoadMatrixf(cam->view);
}
