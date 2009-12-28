#ifndef CE_CAMERA_H
#define CE_CAMERA_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct camera camera;

extern camera* camera_new(void);
extern void camera_delete(camera* cam);

extern float* camera_eye(float eye[3], const camera* cam);
extern float* camera_dir(float dir[3], const camera* cam);
extern float* camera_up(float up[3], const camera* cam);
extern float* camera_right(float right[3], const camera* cam);

extern void camera_set_eye(const float eye[3], camera* cam);
extern void camera_set_look(const float look[4], camera* cam);

extern void camera_move(const float vec[3], camera* cam);
extern void camera_move_rel(const float vec[3], camera* cam);

extern void camera_yaw_pitch(float psi, float theta, camera* cam);

extern void camera_setup(camera* cam);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_CAMERA_H */
