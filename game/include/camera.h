#ifndef CE_CAMERA_H
#define CE_CAMERA_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct camera camera;

extern camera* camera_new(void);
extern void camera_delete(camera* cam);

extern float* camera_eye(float* eye, const camera* cam);
extern float* camera_forward(float* forward, const camera* cam);
extern float* camera_up(float* up, const camera* cam);
extern float* camera_right(float* right, const camera* cam);

extern void camera_set_fov(double fov, camera* cam);
extern void camera_set_aspect(int width, int height, camera* cam);
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
