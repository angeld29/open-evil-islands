#ifndef CE_CAMERA_H
#define CE_CAMERA_H

#include "vec3fwd.h"
#include "quatfwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct camera camera;

extern camera* camera_open(void);
extern void camera_close(camera* cam);

extern float camera_get_fov(camera* cam);
extern float camera_get_aspect(camera* cam);
extern float camera_get_near(camera* cam);
extern float camera_get_far(camera* cam);

extern vec3* camera_get_eye(vec3* eye, camera* cam);
extern vec3* camera_get_forward(vec3* forward, camera* cam);
extern vec3* camera_get_up(vec3* up, camera* cam);
extern vec3* camera_get_right(vec3* right, camera* cam);

extern void camera_set_fov(float fov, camera* cam);
extern void camera_set_aspect(int width, int height, camera* cam);
extern void camera_set_near(float near, camera* cam);
extern void camera_set_far(float far, camera* cam);

extern void camera_set_eye(const vec3* eye, camera* cam);
extern void camera_set_look(const quat* look, camera* cam);

extern void camera_move(float offsetx, float offsetz, camera* cam);
extern void camera_zoom(float offset, camera* cam);
extern void camera_yaw_pitch(float psi, float theta, camera* cam);

extern void camera_setup(camera* cam);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CAMERA_H */
