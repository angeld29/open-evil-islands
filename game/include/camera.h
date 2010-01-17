#ifndef CE_CAMERA_H
#define CE_CAMERA_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct camera camera;

extern camera* camera_new(void);
extern void camera_delete(camera* cam);

extern float camera_get_fov(camera* cam);
extern float camera_get_aspect(camera* cam);
extern float camera_get_near(camera* cam);
extern float camera_get_far(camera* cam);

extern float* camera_get_eye(float* eye, camera* cam);
extern float* camera_get_forward(float* forward, camera* cam);
extern float* camera_get_up(float* up, camera* cam);
extern float* camera_get_right(float* right, camera* cam);

extern void camera_set_fov(float fov, camera* cam);
extern void camera_set_aspect(int width, int height, camera* cam);
extern void camera_set_near(float near, camera* cam);
extern void camera_set_far(float far, camera* cam);

extern void camera_set_eye(const float* eye, camera* cam);
extern void camera_set_look(const float* look, camera* cam);

extern void camera_move(float offsetx, float offsetz, camera* cam);
extern void camera_zoom(float offset, camera* cam);
extern void camera_yaw_pitch(float psi, float theta, camera* cam);

extern void camera_setup(camera* cam);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CAMERA_H */
