#ifndef CE_TEXTURE_H
#define CE_TEXTURE_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct texture texture;

extern texture* texture_open(void* data);
extern void texture_close(texture* tex);

extern void texture_bind(texture* tex);
extern void texture_unbind(texture* tex);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_TEXTURE_H */
